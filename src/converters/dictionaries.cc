/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/converters/dictionaries.h"

#include <nan.h>
#include <webrtc/api/datachannelinterface.h>
#include <webrtc/api/jsep.h>
#include <webrtc/api/peerconnectioninterface.h>
#include <webrtc/api/rtcerror.h>
#include <webrtc/api/rtpparameters.h>
#include <webrtc/api/rtpreceiverinterface.h>
#include <webrtc/api/rtptransceiverinterface.h>
#include <webrtc/api/stats/rtcstats.h>
#include <webrtc/api/video/i420_buffer.h>  // IWYU pragma: keep
#include <v8.h>

#include "src/asyncobjectwrapwithloop.h"  // IWYU pragma: keep
#include "src/converters.h"
#include "src/converters/interfaces.h"  // IWYU pragma: keep
#include "src/converters/object.h"  // IWYU pragma: keep
#include "src/converters/v8.h"  // IWYU pragma: keep
#include "src/functional/either.h"  // IWYU pragma: keep
#include "src/i420helpers.h"  // IWYU pragma: keep
#include "src/errorfactory.h"  // IWYU pragma: keep
#include "src/mediastream.h"  // IWYU pragma: keep
#include "src/mediastreamtrack.h"  // IWYU pragma: keep
#include "src/rtcrtpreceiver.h"  // IWYU pragma: keep
#include "src/rtcrtpsender.h"  // IWYU pragma: keep
#include "src/rtcrtptransceiver.h"  // IWYU pragma: keep
#include "src/rtcstatsresponse.h"  // IWYU pragma: keep

using node_webrtc::BinaryType;  // *
using node_webrtc::RTCAnswerOptions;  // *
using node_webrtc::RTCDtlsFingerprint;  // *
using node_webrtc::RTCIceCredentialType;  // *
using node_webrtc::RTCOAuthCredential;  // *
using node_webrtc::RTCOfferOptions;  // *
using node_webrtc::RTCPeerConnectionState;  // *
using node_webrtc::RTCPriorityType;  // *
using node_webrtc::RTCSdpType;  // *
using node_webrtc::RTCSessionDescriptionInit;  // *
using node_webrtc::RTCVideoSourceInit;  // *
using node_webrtc::UnsignedShortRange;  // *

typedef node_webrtc::Either<std::vector<std::string>, std::string> stringOrStrings;
typedef node_webrtc::Either<std::string, node_webrtc::RTCOAuthCredential> stringOrCredential;

#define EXPAND_OBJ_FROM_JS_DEFAULT(TYPE, NAME, DEFAULT) * GetOptional<TYPE>(object, NAME, DEFAULT)

#define EXPAND_OBJ_FROM_JS_OPTIONAL(TYPE, NAME) * GetOptional<TYPE>(object, NAME)

#define EXPAND_OBJ_FROM_JS_REQUIRED(TYPE, NAME) * GetRequired<TYPE>(object, NAME)

// TODO(mroberts): Use CONVERT_VIA and go through v8::Local<v8::Object>?
// TODO(mroberts): Explain when to use _IMPL1 versus _IMPL2.
// TODO(mroberts): Correct usage of `T` versus `TYPE`, etc., in comments.

#define OBJ_FROM_JS_IMPL1(TYPE, FN) \
  FROM_JS_IMPL(TYPE, value) { \
    return node_webrtc::From<v8::Local<v8::Object>>(value).FlatMap<TYPE>( \
    [](const v8::Local<v8::Object> object) { \
      return node_webrtc::Pure(curry(FN)) \
          TYPE ## _LIST \
          ; \
    }); \
  }

#define OBJ_FROM_JS_IMPL2(TYPE, FN) \
  FROM_JS_IMPL(TYPE, value) { \
    return node_webrtc::From<v8::Local<v8::Object>>(value).FlatMap<TYPE>( \
    [](const v8::Local<v8::Object> object) { \
      return node_webrtc::Validation<TYPE>::Join( \
              node_webrtc::Pure(curry(FN)) \
              TYPE ## _LIST \
          ); \
    }); \
  }

static node_webrtc::RTCOAuthCredential CreateRTCOAuthCredential(
    const std::string& macKey,
    const std::string& accessToken) {
  return {macKey, accessToken};
}

static node_webrtc::Validation<webrtc::PeerConnectionInterface::IceServer> CreateIceServer(
    const node_webrtc::Either<std::vector<std::string>, std::string>& urlsOrUrl,
    const std::string& username,
    const node_webrtc::Either<std::string, node_webrtc::RTCOAuthCredential>& credential,
    const node_webrtc::RTCIceCredentialType credentialType) {
  if (credential.IsRight() || credentialType != node_webrtc::RTCIceCredentialType::kPassword) {
    return node_webrtc::Validation<webrtc::PeerConnectionInterface::IceServer>::Invalid("OAuth is not currently supported");
  }
  webrtc::PeerConnectionInterface::IceServer iceServer;
  iceServer.urls = urlsOrUrl.FromLeft(std::vector<std::string>());
  iceServer.uri = urlsOrUrl.FromRight("");
  iceServer.username = username;
  iceServer.password = credential.UnsafeFromLeft();
  return node_webrtc::Pure(iceServer);
}

TO_JS_IMPL(webrtc::PeerConnectionInterface::IceServer, iceServer) {
  Nan::EscapableHandleScope scope;
  auto object = Nan::New<v8::Object>();
  if (!iceServer.uri.empty()) {
    object->Set(Nan::New("urls").ToLocalChecked(), Nan::New(iceServer.uri).ToLocalChecked());
  } else {
    auto maybeArray = node_webrtc::From<v8::Local<v8::Value>>(iceServer.urls);
    if (maybeArray.IsInvalid()) {
      return node_webrtc::Validation<v8::Local<v8::Value>>::Invalid(maybeArray.ToErrors());
    }
    object->Set(Nan::New("urls").ToLocalChecked(), maybeArray.UnsafeFromValid());
  }
  if (!iceServer.username.empty()) {
    object->Set(Nan::New("username").ToLocalChecked(), Nan::New(iceServer.username).ToLocalChecked());
  }
  if (!iceServer.password.empty()) {
    object->Set(Nan::New("credential").ToLocalChecked(), Nan::New(iceServer.password).ToLocalChecked());
    object->Set(Nan::New("credentialType").ToLocalChecked(), Nan::New("password").ToLocalChecked());
  }
  return node_webrtc::Pure(scope.Escape(object.As<v8::Value>()));
}

static node_webrtc::RTCDtlsFingerprint CreateRTCDtlsFingerprint(
    const node_webrtc::Maybe<std::string>& algorithm,
    const node_webrtc::Maybe<std::string>& value) {
  return {algorithm, value};
}

static node_webrtc::Validation<node_webrtc::UnsignedShortRange> CreateUnsignedShortRange(
    const node_webrtc::Maybe<uint16_t>& maybeMin,
    const node_webrtc::Maybe<uint16_t>& maybeMax) {
  auto min = maybeMin.FromMaybe(0);
  auto max = maybeMax.FromMaybe(65535);
  if (min > max) {
    return node_webrtc::Validation<node_webrtc::UnsignedShortRange>::Invalid("Expected min to be less than max");
  }
  return node_webrtc::Pure<node_webrtc::UnsignedShortRange>({maybeMin, maybeMax});
}

TO_JS_IMPL(node_webrtc::UnsignedShortRange, value) {
  Nan::EscapableHandleScope scope;
  auto object = Nan::New<v8::Object>();
  if (value.min.IsJust()) {
    object->Set(Nan::New("min").ToLocalChecked(), Nan::New(value.min.UnsafeFromJust()));
  }
  if (value.max.IsJust()) {
    object->Set(Nan::New("max").ToLocalChecked(), Nan::New(value.max.UnsafeFromJust()));
  }
  return node_webrtc::Pure(scope.Escape(object.As<v8::Value>()));
}

static webrtc::PeerConnectionInterface::RTCConfiguration CreateRTCConfiguration(
    const std::vector<webrtc::PeerConnectionInterface::IceServer>& iceServers,
    const webrtc::PeerConnectionInterface::IceTransportsType iceTransportsPolicy,
    const webrtc::PeerConnectionInterface::BundlePolicy bundlePolicy,
    const webrtc::PeerConnectionInterface::RtcpMuxPolicy rtcpMuxPolicy,
    const node_webrtc::Maybe<std::string>&,
    const node_webrtc::Maybe<std::vector<v8::Local<v8::Object>>>&,
    const uint32_t iceCandidatePoolSize,
    const webrtc::SdpSemantics sdpSemantics) {
  webrtc::PeerConnectionInterface::RTCConfiguration configuration;
  configuration.servers = iceServers;
  configuration.type = iceTransportsPolicy;
  configuration.bundle_policy = bundlePolicy;
  configuration.rtcp_mux_policy = rtcpMuxPolicy;
  configuration.ice_candidate_pool_size = iceCandidatePoolSize;
  configuration.sdp_semantics = sdpSemantics;
  return configuration;
}

FROM_JS_IMPL(webrtc::PeerConnectionInterface::RTCConfiguration, value) {
  // NOTE(mroberts): Allow overriding the default SdpSemantics via environment variable.
  // Makes web-platform-tests easier to run.
  auto sdp_semantics_env = std::getenv("SDP_SEMANTICS");
  auto sdp_semantics_str = sdp_semantics_env ? std::string(sdp_semantics_env) : std::string();
  auto sdp_semantics = node_webrtc::From<webrtc::SdpSemantics>(sdp_semantics_str).FromValidation(webrtc::SdpSemantics::kPlanB);
  return node_webrtc::From<v8::Local<v8::Object>>(value).FlatMap<webrtc::PeerConnectionInterface::RTCConfiguration>(
  [sdp_semantics](const v8::Local<v8::Object> object) {
    return curry(CreateRTCConfiguration)
        % node_webrtc::GetOptional<std::vector<webrtc::PeerConnectionInterface::IceServer>>(object, "iceServers", std::vector<webrtc::PeerConnectionInterface::IceServer>())
        * node_webrtc::GetOptional<webrtc::PeerConnectionInterface::IceTransportsType>(object, "iceTransportPolicy", webrtc::PeerConnectionInterface::IceTransportsType::kAll)
        * node_webrtc::GetOptional<webrtc::PeerConnectionInterface::BundlePolicy>(object, "bundlePolicy", webrtc::PeerConnectionInterface::BundlePolicy::kBundlePolicyBalanced)
        * node_webrtc::GetOptional<webrtc::PeerConnectionInterface::RtcpMuxPolicy>(object, "rtcpMuxPolicy", webrtc::PeerConnectionInterface::RtcpMuxPolicy::kRtcpMuxPolicyRequire)
        * node_webrtc::GetOptional<std::string>(object, "peerIdentity")
        * node_webrtc::GetOptional<std::vector<v8::Local<v8::Object>>>(object, "certificates")
        // TODO(mroberts): Implement EnforceRange and change to uint8_t.
        * node_webrtc::GetOptional<uint8_t>(object, "iceCandidatePoolSize", 0)
        * node_webrtc::GetOptional<webrtc::SdpSemantics>(object, "sdpSemantics", sdp_semantics);
  });
}

static node_webrtc::ExtendedRTCConfiguration CreateExtendedRTCConfiguration(
    const webrtc::PeerConnectionInterface::RTCConfiguration configuration,
    const node_webrtc::UnsignedShortRange portRange) {
  return node_webrtc::ExtendedRTCConfiguration(configuration, portRange);
}

FROM_JS_IMPL(node_webrtc::ExtendedRTCConfiguration, value) {
  return node_webrtc::From<v8::Local<v8::Object>>(value).FlatMap<node_webrtc::ExtendedRTCConfiguration>(
  [](const v8::Local<v8::Object> object) {
    return curry(CreateExtendedRTCConfiguration)
        % node_webrtc::From<webrtc::PeerConnectionInterface::RTCConfiguration>(static_cast<v8::Local<v8::Value>>(object))
        * node_webrtc::GetOptional<node_webrtc::UnsignedShortRange>(object, "portRange", node_webrtc::UnsignedShortRange());
  });
}

static v8::Local<v8::Value> ExtendedRTCConfigurationToJavaScript(
    const v8::Local<v8::Value> iceServers,
    const v8::Local<v8::Value> iceTransportPolicy,
    const v8::Local<v8::Value> bundlePolicy,
    const v8::Local<v8::Value> rtcpMuxPolicy,
    const v8::Local<v8::Value> iceCandidatePoolSize,
    const v8::Local<v8::Value> portRange,
    const v8::Local<v8::Value> sdpSemantics) {
  Nan::EscapableHandleScope scope;
  auto object = Nan::New<v8::Object>();
  object->Set(Nan::New("iceServers").ToLocalChecked(), iceServers);
  object->Set(Nan::New("iceTransportPolicy").ToLocalChecked(), iceTransportPolicy);
  object->Set(Nan::New("bundlePolicy").ToLocalChecked(), bundlePolicy);
  object->Set(Nan::New("rtcpMuxPolicy").ToLocalChecked(), rtcpMuxPolicy);
  object->Set(Nan::New("iceCandidatePoolSize").ToLocalChecked(), iceCandidatePoolSize);
  object->Set(Nan::New("portRange").ToLocalChecked(), portRange);
  object->Set(Nan::New("sdpSemantics").ToLocalChecked(), sdpSemantics);
  return scope.Escape(object);
}

TO_JS_IMPL(node_webrtc::ExtendedRTCConfiguration, configuration) {
  return curry(ExtendedRTCConfigurationToJavaScript)
      % node_webrtc::From<v8::Local<v8::Value>>(configuration.configuration.servers)
      * node_webrtc::From<v8::Local<v8::Value>>(configuration.configuration.type)
      * node_webrtc::From<v8::Local<v8::Value>>(configuration.configuration.bundle_policy)
      * node_webrtc::From<v8::Local<v8::Value>>(configuration.configuration.rtcp_mux_policy)
      * node_webrtc::Pure(Nan::New(configuration.configuration.ice_candidate_pool_size))
      * node_webrtc::From<v8::Local<v8::Value>>(configuration.portRange)
      * node_webrtc::From<v8::Local<v8::Value>>(configuration.configuration.sdp_semantics);
}

static node_webrtc::RTCOfferOptions CreateRTCOfferOptions(
    const bool voiceActivityDetection,
    const bool iceRestart,
    const node_webrtc::Maybe<bool> offerToReceiveAudio,
    const node_webrtc::Maybe<bool> offerToReceiveVideo) {
  webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
  options.ice_restart = iceRestart;
  options.voice_activity_detection = voiceActivityDetection;
  options.offer_to_receive_audio = offerToReceiveAudio.Map(
  [](const bool boolean) { return boolean ? webrtc::PeerConnectionInterface::RTCOfferAnswerOptions::kOfferToReceiveMediaTrue : 0; }
      ).FromMaybe(webrtc::PeerConnectionInterface::RTCOfferAnswerOptions::kUndefined);
  options.offer_to_receive_video = offerToReceiveVideo.Map(
  [](const bool boolean) { return boolean ? webrtc::PeerConnectionInterface::RTCOfferAnswerOptions::kOfferToReceiveMediaTrue : 0; }
      ).FromMaybe(webrtc::PeerConnectionInterface::RTCOfferAnswerOptions::kUndefined);
  return node_webrtc::RTCOfferOptions(options);
}

static node_webrtc::RTCAnswerOptions CreateRTCAnswerOptions(const bool voiceActivityDetection) {
  webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
  options.voice_activity_detection = voiceActivityDetection;
  return node_webrtc::RTCAnswerOptions(options);
}

static node_webrtc::RTCSessionDescriptionInit CreateRTCSessionDescriptionInit(
    const node_webrtc::RTCSdpType type,
    const std::string sdp) {
  return {type, sdp};
}

static node_webrtc::RTCVideoSourceInit CreateRTCVideoSourceInit(
    const bool isScreencast,
    const node_webrtc::Maybe<bool> needsDenoising
) {
  return {isScreencast, needsDenoising};
}

node_webrtc::Validation<webrtc::SessionDescriptionInterface*> node_webrtc::Converter<node_webrtc::RTCSessionDescriptionInit, webrtc::SessionDescriptionInterface*>::Convert(
    const node_webrtc::RTCSessionDescriptionInit init) {
  std::string type_;
  switch (init.type) {
    case node_webrtc::RTCSdpType::kOffer:
      type_ = "offer";
      break;
    case node_webrtc::RTCSdpType::kPrAnswer:
      type_ = "pranswer";
      break;
    case node_webrtc::RTCSdpType::kAnswer:
      type_ = "answer";
      break;
    case node_webrtc::RTCSdpType::kRollback:
      return node_webrtc::Validation<webrtc::SessionDescriptionInterface*>::Invalid("Rollback is not currently supported");
  }
  webrtc::SdpParseError error;
  auto description = webrtc::CreateSessionDescription(type_, init.sdp, &error);
  if (!description) {
    return node_webrtc::Validation<webrtc::SessionDescriptionInterface*>::Invalid(error.description);
  }
  return node_webrtc::Pure(description);
}

TO_JS_IMPL(node_webrtc::RTCSessionDescriptionInit, init) {
  Nan::EscapableHandleScope scope;
  auto maybeType = node_webrtc::From<std::string>(init.type);
  if (maybeType.IsInvalid()) {
    return node_webrtc::Validation<v8::Local<v8::Value>>::Invalid(maybeType.ToErrors()[0]);
  }
  auto object = Nan::New<v8::Object>();
  object->Set(Nan::New("sdp").ToLocalChecked(), Nan::New(init.sdp).ToLocalChecked());
  object->Set(Nan::New("type").ToLocalChecked(), Nan::New(maybeType.UnsafeFromValid()).ToLocalChecked());
  return node_webrtc::Pure(scope.Escape(object.As<v8::Value>()));
}

node_webrtc::Validation<node_webrtc::RTCSessionDescriptionInit> node_webrtc::Converter<webrtc::SessionDescriptionInterface*, node_webrtc::RTCSessionDescriptionInit>::Convert(
    webrtc::SessionDescriptionInterface* description) {
  if (!description) {
    return node_webrtc::Validation<node_webrtc::RTCSessionDescriptionInit>::Invalid("RTCSessionDescription is null");
  }
  std::string sdp;
  if (!description->ToString(&sdp)) {
    return node_webrtc::Validation<node_webrtc::RTCSessionDescriptionInit>::Invalid("Failed to print the SDP. This is pretty weird. File a bug on https://github.com/js-platform/node-webrtc");
  }
  return curry(CreateRTCSessionDescriptionInit)
      % node_webrtc::From<node_webrtc::RTCSdpType>(description->type())
      * node_webrtc::Validation<std::string>(sdp);
}

FROM_JS_IMPL(webrtc::SessionDescriptionInterface*, value) {
  return node_webrtc::From<node_webrtc::RTCSessionDescriptionInit>(value)
      .FlatMap<webrtc::SessionDescriptionInterface*>(node_webrtc::Converter<node_webrtc::RTCSessionDescriptionInit, webrtc::SessionDescriptionInterface*>::Convert);
}

TO_JS_IMPL(const webrtc::SessionDescriptionInterface*, value) {
  Nan::EscapableHandleScope scope;

  if (!value) {
    return node_webrtc::Validation<v8::Local<v8::Value>>::Invalid("RTCSessionDescription is null");
  }

  std::string sdp;
  if (!value->ToString(&sdp)) {
    return node_webrtc::Validation<v8::Local<v8::Value>>::Invalid("Failed to print the SDP. This is pretty weird. File a bug on https://github.com/js-platform/node-webrtc");
  }

  auto object = Nan::New<v8::Object>();
  object->Set(Nan::New("sdp").ToLocalChecked(), Nan::New(sdp).ToLocalChecked());
  object->Set(Nan::New("type").ToLocalChecked(), Nan::New(value->type()).ToLocalChecked());

  return node_webrtc::Pure(scope.Escape(object.As<v8::Value>()));
}

static node_webrtc::Validation<webrtc::IceCandidateInterface*> CreateIceCandidateInterface(
    const std::string& candidate,
    const std::string& sdpMid,
    const int sdpMLineIndex,
    const node_webrtc::Maybe<std::string>&) {
  webrtc::SdpParseError error;
  auto candidate_ = webrtc::CreateIceCandidate(sdpMid, sdpMLineIndex, candidate, &error);
  if (!candidate_) {
    return node_webrtc::Validation<webrtc::IceCandidateInterface*>::Invalid(error.description);
  }
  return node_webrtc::Pure(candidate_);
}

TO_JS_IMPL(const webrtc::IceCandidateInterface*, value) {
  Nan::EscapableHandleScope scope;

  if (!value) {
    return node_webrtc::Validation<v8::Local<v8::Value>>::Invalid("RTCIceCandidate is null");
  }

  std::string candidate;
  if (!value->ToString(&candidate)) {
    return node_webrtc::Validation<v8::Local<v8::Value>>::Invalid("Failed to print the candidate string. This is pretty weird. File a bug on https://github.com/js-platform/node-webrtc");
  }

  auto object = Nan::New<v8::Object>();
  object->Set(Nan::New("candidate").ToLocalChecked(), Nan::New(candidate).ToLocalChecked());
  object->Set(Nan::New("sdpMid").ToLocalChecked(), Nan::New(value->sdp_mid()).ToLocalChecked());
  object->Set(Nan::New("sdpMLineIndex").ToLocalChecked(), Nan::New(value->sdp_mline_index()));

  return node_webrtc::Pure(scope.Escape(object.As<v8::Value>()));
}

static node_webrtc::Validation<webrtc::DataChannelInit> CreateDataChannelInit(
    const bool ordered,
    const node_webrtc::Maybe<uint32_t> maxPacketLifeTime,
    const node_webrtc::Maybe<uint32_t> maxRetransmits,
    const std::string& protocol,
    const bool negotiated,
    const node_webrtc::Maybe<uint32_t> id,
    const node_webrtc::RTCPriorityType) {
  if (id.FromMaybe(0) > UINT16_MAX) {
    return node_webrtc::Validation<webrtc::DataChannelInit>::Invalid("id must be between 0 and 65534, inclusive");
  } else if (maxPacketLifeTime.IsJust() && maxRetransmits.IsJust()) {
    return node_webrtc::Validation<webrtc::DataChannelInit>::Invalid("You cannot set both maxPacketLifeTime and maxRetransmits");
  }
  webrtc::DataChannelInit init;
  init.ordered = ordered;
  init.maxRetransmitTime = maxPacketLifeTime.Map([](const uint32_t i) { return static_cast<int>(i); }).FromMaybe(-1);
  init.maxRetransmits = maxRetransmits.Map([](const uint32_t i) { return static_cast<int>(i); }).FromMaybe(-1);
  init.protocol = protocol;
  init.negotiated = negotiated;
  init.id = id.Map([](const uint32_t i) { return static_cast<int>(i); }).FromMaybe(-1);
  return node_webrtc::Pure(init);
}

TO_JS_IMPL(webrtc::RTCError*, error) {
  return node_webrtc::Converter<const webrtc::RTCError*, v8::Local<v8::Value>>::Convert(error);
}

TO_JS_IMPL(const webrtc::RTCError*, error) {
  Nan::EscapableHandleScope scope;
  if (!error) {
    return node_webrtc::Validation<v8::Local<v8::Value>>::Invalid("No error? Please file a bug on https://github.com/js-platform/node-webrtc");
  }
  switch (error->type()) {
    case webrtc::RTCErrorType::NONE:
      return node_webrtc::Validation<v8::Local<v8::Value>>::Invalid("No error? Please file a bug on https://github.com/js-platform/node-webrtc");
    case webrtc::RTCErrorType::UNSUPPORTED_PARAMETER:
    case webrtc::RTCErrorType::INVALID_PARAMETER:
      return node_webrtc::Pure(scope.Escape(node_webrtc::ErrorFactory::CreateInvalidAccessError(error->message())));
    case webrtc::RTCErrorType::INVALID_RANGE:
      return node_webrtc::Pure(scope.Escape(node_webrtc::ErrorFactory::CreateRangeError(error->message())));
    case webrtc::RTCErrorType::SYNTAX_ERROR:
      return node_webrtc::Pure(scope.Escape(node_webrtc::ErrorFactory::CreateSyntaxError(error->message())));
    case webrtc::RTCErrorType::INVALID_STATE:
      return node_webrtc::Pure(scope.Escape(node_webrtc::ErrorFactory::CreateInvalidStateError(error->message())));
    case webrtc::RTCErrorType::INVALID_MODIFICATION:
      return node_webrtc::Pure(scope.Escape(node_webrtc::ErrorFactory::CreateInvalidModificationError(error->message())));
    case webrtc::RTCErrorType::NETWORK_ERROR:
      return node_webrtc::Pure(scope.Escape(node_webrtc::ErrorFactory::CreateNetworkError(error->message())));
    // NOTE(mroberts): SetLocalDescription in the wrong state can throw this.
    case webrtc::RTCErrorType::INTERNAL_ERROR:
      return node_webrtc::Pure(scope.Escape(node_webrtc::ErrorFactory::CreateInvalidStateError(error->message())));
    case webrtc::RTCErrorType::UNSUPPORTED_OPERATION:
    case webrtc::RTCErrorType::RESOURCE_EXHAUSTED:
      return node_webrtc::Pure(scope.Escape(node_webrtc::ErrorFactory::CreateOperationError(error->message())));
  }
}

node_webrtc::Validation<node_webrtc::SomeError> node_webrtc::Converter<webrtc::RTCError*, node_webrtc::SomeError>::Convert(webrtc::RTCError* error) {
  return node_webrtc::Converter<const webrtc::RTCError*, node_webrtc::SomeError>::Convert(error);
}

node_webrtc::Validation<node_webrtc::SomeError> node_webrtc::Converter<const webrtc::RTCError*, node_webrtc::SomeError>::Convert(const webrtc::RTCError* error) {
  if (!error) {
    return node_webrtc::Validation<node_webrtc::SomeError>::Invalid("No error? Please file a bug on https://github.com/js-platform/node-webrtc");
  }
  auto type = node_webrtc::MakeRight<node_webrtc::ErrorFactory::DOMExceptionName>(node_webrtc::ErrorFactory::ErrorName::kError);
  switch (error->type()) {
    case webrtc::RTCErrorType::NONE:
      return node_webrtc::Validation<node_webrtc::SomeError>::Invalid("No error? Please file a bug on https://github.com/js-platform/node-webrtc");
    case webrtc::RTCErrorType::UNSUPPORTED_PARAMETER:
    case webrtc::RTCErrorType::INVALID_PARAMETER:
      type = node_webrtc::MakeLeft<node_webrtc::ErrorFactory::ErrorName>(node_webrtc::ErrorFactory::DOMExceptionName::kInvalidAccessError);
      break;
    case webrtc::RTCErrorType::INVALID_RANGE:
      type = node_webrtc::MakeRight<node_webrtc::ErrorFactory::DOMExceptionName>(node_webrtc::ErrorFactory::ErrorName::kRangeError);
      break;
    case webrtc::RTCErrorType::SYNTAX_ERROR:
      type = node_webrtc::MakeRight<node_webrtc::ErrorFactory::DOMExceptionName>(node_webrtc::ErrorFactory::ErrorName::kSyntaxError);
      break;
    case webrtc::RTCErrorType::INVALID_STATE:
      type = node_webrtc::MakeLeft<node_webrtc::ErrorFactory::ErrorName>(node_webrtc::ErrorFactory::DOMExceptionName::kInvalidStateError);
      break;
    case webrtc::RTCErrorType::INVALID_MODIFICATION:
      type = node_webrtc::MakeLeft<node_webrtc::ErrorFactory::ErrorName>(node_webrtc::ErrorFactory::DOMExceptionName::kInvalidModificationError);
      break;
    case webrtc::RTCErrorType::NETWORK_ERROR:
      type = node_webrtc::MakeLeft<node_webrtc::ErrorFactory::ErrorName>(node_webrtc::ErrorFactory::DOMExceptionName::kNetworkError);
      break;
    // NOTE(mroberts): SetLocalDescription in the wrong state can throw this.
    case webrtc::RTCErrorType::INTERNAL_ERROR:
      type = node_webrtc::MakeLeft<node_webrtc::ErrorFactory::ErrorName>(node_webrtc::ErrorFactory::DOMExceptionName::kInvalidStateError);
      break;
    case webrtc::RTCErrorType::UNSUPPORTED_OPERATION:
    case webrtc::RTCErrorType::RESOURCE_EXHAUSTED:
      type = node_webrtc::MakeLeft<node_webrtc::ErrorFactory::ErrorName>(node_webrtc::ErrorFactory::DOMExceptionName::kOperationError);
      break;
  }
  return node_webrtc::Pure(node_webrtc::SomeError(error->message(), type));
}

node_webrtc::Validation<node_webrtc::RTCPeerConnectionState> node_webrtc::Converter<webrtc::PeerConnectionInterface::IceConnectionState, node_webrtc::RTCPeerConnectionState>::Convert(webrtc::PeerConnectionInterface::IceConnectionState state) {
  switch (state) {
    case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionNew:
      return node_webrtc::Pure(kNew);
    case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionChecking:
      return node_webrtc::Pure(kConnecting);
    case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionConnected:
    case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionCompleted:
      return node_webrtc::Pure(kConnected);
    case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionDisconnected:
      return node_webrtc::Pure(kDisconnected);
    case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionFailed:
      return node_webrtc::Pure(kFailed);
    case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionClosed:
      return node_webrtc::Pure(kClosed);
    case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionMax:
      return node_webrtc::Validation<node_webrtc::RTCPeerConnectionState>::Invalid(
              "WebRTC\'s RTCPeerConnection has an ICE connection state \"max\", but I have no idea"
              "what this means. If you see this error, file a bug on https://github.com/js-platform/node-webrtc");
  }
}

TO_JS_IMPL(node_webrtc::RTCStatsResponseInit, init) {
  Nan::EscapableHandleScope scope;
  return node_webrtc::Pure(scope.Escape(node_webrtc::RTCStatsResponse::Create(init.first, init.second)->handle().As<v8::Value>()));
}

TO_JS_IMPL(webrtc::RtpSource, source) {
  Nan::EscapableHandleScope scope;
  auto object = Nan::New<v8::Object>();
  object->Set(Nan::New("timestamp").ToLocalChecked(), Nan::New<v8::Number>(source.timestamp_ms()));
  object->Set(Nan::New("source").ToLocalChecked(), Nan::New(source.source_id()));
  return node_webrtc::Pure(scope.Escape(object).As<v8::Value>());
}

TO_JS_IMPL(webrtc::RtpHeaderExtensionParameters, params) {
  Nan::EscapableHandleScope scope;
  auto object = Nan::New<v8::Object>();
  object->Set(Nan::New("uri").ToLocalChecked(), Nan::New(params.uri).ToLocalChecked());
  object->Set(Nan::New("id").ToLocalChecked(), Nan::New(params.id));
  return node_webrtc::Pure(scope.Escape(object.As<v8::Value>()));
}

TO_JS_IMPL(webrtc::RtpCodecParameters, params) {
  Nan::EscapableHandleScope scope;
  auto object = Nan::New<v8::Object>();
  object->Set(Nan::New("payloadType").ToLocalChecked(), Nan::New(params.payload_type));
  object->Set(Nan::New("mimeType").ToLocalChecked(), Nan::New(params.mime_type()).ToLocalChecked());
  if (params.clock_rate) {
    object->Set(Nan::New("clockRate").ToLocalChecked(), Nan::New(*params.clock_rate));
  }
  if (params.num_channels) {
    object->Set(Nan::New("channels").ToLocalChecked(), Nan::New(*params.num_channels));
  }
  if (!params.parameters.empty()) {
    std::string fmtp("a=fmtp:" + std::to_string(params.payload_type));
    unsigned long i = 0;
    for (auto param : params.parameters) {
      fmtp += " " + param.first + "=" + param.second;
      if (i < params.parameters.size() - 1) {
        fmtp += ";";
      }
      i++;
    }
    object->Set(Nan::New("sdpFmtpLine").ToLocalChecked(), Nan::New(fmtp).ToLocalChecked());
  }
  return node_webrtc::Pure(scope.Escape(object).As<v8::Value>());
}

TO_JS_IMPL(webrtc::RtcpParameters, params) {
  Nan::EscapableHandleScope scope;
  auto object = Nan::New<v8::Object>();
  if (!params.cname.empty()) {
    object->Set(Nan::New("cname").ToLocalChecked(), Nan::New(params.cname).ToLocalChecked());
  }
  object->Set(Nan::New("reducedSize").ToLocalChecked(), Nan::New(params.reduced_size));
  return node_webrtc::Pure(scope.Escape(object.As<v8::Value>()));
}

static v8::Local<v8::Value> CreateRtpParameters(v8::Local<v8::Value> headerExtensions, v8::Local<v8::Value> codecs, v8::Local<v8::Value> rtcp) {
  Nan::EscapableHandleScope scope;
  auto object = Nan::New<v8::Object>();
  object->Set(Nan::New("headerExtensions").ToLocalChecked(), headerExtensions);
  object->Set(Nan::New("codecs").ToLocalChecked(), codecs);
  object->Set(Nan::New("encodings").ToLocalChecked(), Nan::New<v8::Array>());
  object->Set(Nan::New("rtcp").ToLocalChecked(), rtcp);
  return scope.Escape(object);
}

TO_JS_IMPL(webrtc::RtpParameters, params) {
  return curry(CreateRtpParameters)
      % node_webrtc::From<v8::Local<v8::Value>>(params.header_extensions)
      * node_webrtc::From<v8::Local<v8::Value>>(params.codecs)
      * node_webrtc::From<v8::Local<v8::Value>>(params.rtcp);
}

static webrtc::RtpTransceiverInit CreateRtpTransceiverInit(
    const webrtc::RtpTransceiverDirection direction,
    const std::vector<node_webrtc::MediaStream*> streams) {
  webrtc::RtpTransceiverInit init;
  init.direction = direction;
  std::vector<std::string> stream_ids;
  for (const auto& stream : streams) {
    stream_ids.emplace_back(stream->stream()->id());
  }
  init.stream_ids = stream_ids;
  return init;
}

TO_JS_IMPL(const webrtc::RTCStatsMemberInterface*, value) {
  switch (value->type()) {
    case webrtc::RTCStatsMemberInterface::Type::kBool:  // bool
      return node_webrtc::From<v8::Local<v8::Value>>(*value->cast_to<webrtc::RTCStatsMember<bool>>());
    case webrtc::RTCStatsMemberInterface::Type::kInt32:  // int32_t
      return node_webrtc::From<v8::Local<v8::Value>>(*value->cast_to<webrtc::RTCStatsMember<int32_t>>());
    case webrtc::RTCStatsMemberInterface::Type::kUint32:  // uint32_t
      return node_webrtc::From<v8::Local<v8::Value>>(*value->cast_to<webrtc::RTCStatsMember<uint32_t>>());
    case webrtc::RTCStatsMemberInterface::Type::kInt64:   // int64_t
      return node_webrtc::From<v8::Local<v8::Value>>(*value->cast_to<webrtc::RTCStatsMember<int64_t>>());
    case webrtc::RTCStatsMemberInterface::Type::kUint64:  // uint64_t
      return node_webrtc::From<v8::Local<v8::Value>>(*value->cast_to<webrtc::RTCStatsMember<uint64_t>>());
    case webrtc::RTCStatsMemberInterface::Type::kDouble:  // double
      return node_webrtc::From<v8::Local<v8::Value>>(*value->cast_to<webrtc::RTCStatsMember<double>>());
    case webrtc::RTCStatsMemberInterface::Type::kString:  // std::string
      return node_webrtc::From<v8::Local<v8::Value>>(*value->cast_to<webrtc::RTCStatsMember<std::string>>());
    case webrtc::RTCStatsMemberInterface::Type::kSequenceBool:  // std::vector<bool>
      return node_webrtc::From<v8::Local<v8::Value>>(*value->cast_to<webrtc::RTCStatsMember<std::vector<bool>>>());
    case webrtc::RTCStatsMemberInterface::Type::kSequenceInt32:  // std::vector<int32_t>
      return node_webrtc::From<v8::Local<v8::Value>>(*value->cast_to<webrtc::RTCStatsMember<std::vector<int32_t>>>());
    case webrtc::RTCStatsMemberInterface::Type::kSequenceUint32:  // std::vector<uint32_t>
      return node_webrtc::From<v8::Local<v8::Value>>(*value->cast_to<webrtc::RTCStatsMember<std::vector<uint32_t>>>());
    case webrtc::RTCStatsMemberInterface::Type::kSequenceInt64:  // std::vector<int64_t>
      return node_webrtc::From<v8::Local<v8::Value>>(*value->cast_to<webrtc::RTCStatsMember<std::vector<int64_t>>>());
    case webrtc::RTCStatsMemberInterface::Type::kSequenceUint64:  // std::vector<uint64_t>
      return node_webrtc::From<v8::Local<v8::Value>>(*value->cast_to<webrtc::RTCStatsMember<std::vector<uint64_t>>>());
    case webrtc::RTCStatsMemberInterface::Type::kSequenceDouble:  // std::vector<double>
      return node_webrtc::From<v8::Local<v8::Value>>(*value->cast_to<webrtc::RTCStatsMember<std::vector<double>>>());
    case webrtc::RTCStatsMemberInterface::Type::kSequenceString:  // std::vector<std::string>
      return node_webrtc::From<v8::Local<v8::Value>>(*value->cast_to<webrtc::RTCStatsMember<std::vector<std::string>>>());
  }
}

TO_JS_IMPL(const webrtc::RTCStats*, value) {
  Nan::EscapableHandleScope scope;
  auto stats = Nan::New<v8::Object>();
  stats->Set(Nan::New("id").ToLocalChecked(), node_webrtc::From<v8::Local<v8::Value>>(value->id()).UnsafeFromValid());
  stats->Set(Nan::New("timestamp").ToLocalChecked(), node_webrtc::From<v8::Local<v8::Value>>(value->timestamp_us() / 1000.0).UnsafeFromValid());
  stats->Set(Nan::New("type").ToLocalChecked(), node_webrtc::From<v8::Local<v8::Value>>(std::string(value->type())).UnsafeFromValid());
  for (const webrtc::RTCStatsMemberInterface* member : value->Members()) {
    if (member->is_defined()) {
      stats->Set(Nan::New(member->name()).ToLocalChecked(), node_webrtc::From<v8::Local<v8::Value>>(member).UnsafeFromValid());
    }
  }
  return node_webrtc::Pure(scope.Escape(stats).As<v8::Value>());
}

TO_JS_IMPL(rtc::scoped_refptr<webrtc::RTCStatsReport>, value) {
  Nan::EscapableHandleScope scope;
  auto context = Nan::GetCurrentContext();
  auto report = v8::Map::New(context->GetIsolate());
  for (const webrtc::RTCStats& stats : *value) {
    report->Set(context, Nan::New(stats.id()).ToLocalChecked(), node_webrtc::From<v8::Local<v8::Value>>(&stats).UnsafeFromValid()).IsEmpty();
  }
  return node_webrtc::Pure(scope.Escape(report).As<v8::Value>());
}

namespace node_webrtc {

DECLARE_CONVERTER(v8::Local<v8::Value>, node_webrtc::ImageData)
CONVERTER_IMPL(v8::Local<v8::Value>, node_webrtc::ImageData, value) {
  return node_webrtc::From<v8::Local<v8::Object>>(value).FlatMap<node_webrtc::ImageData>(
  [](const v8::Local<v8::Object> object) {
    return curry(node_webrtc::ImageData::Create)
        % node_webrtc::GetRequired<int>(object, "width")
        * node_webrtc::GetRequired<int>(object, "height")
        * node_webrtc::GetRequired<v8::ArrayBuffer::Contents>(object, "data");
  });
}

DECLARE_CONVERTER(node_webrtc::ImageData, I420ImageData)
CONVERTER_IMPL(node_webrtc::ImageData, I420ImageData, imageData) {
  return imageData.toI420();
}

CONVERT_VIA(v8::Local<v8::Value>, node_webrtc::ImageData, I420ImageData)

DECLARE_CONVERTER(node_webrtc::ImageData, RgbaImageData)
CONVERTER_IMPL(node_webrtc::ImageData, RgbaImageData, imageData) {
  return imageData.toRgba();
}

CONVERT_VIA(v8::Local<v8::Value>, node_webrtc::ImageData, RgbaImageData)

DECLARE_CONVERTER(node_webrtc::I420ImageData, rtc::scoped_refptr<webrtc::I420Buffer>)
CONVERT_VIA(v8::Local<v8::Value>, node_webrtc::I420ImageData, rtc::scoped_refptr<webrtc::I420Buffer>)

}  // namespace node_webrtc

static rtc::scoped_refptr<webrtc::I420Buffer> CreateI420Buffer(
    node_webrtc::I420ImageData i420Frame
) {
  auto buffer = webrtc::I420Buffer::Create(i420Frame.width(), i420Frame.height());
  memcpy(buffer->MutableDataY(), i420Frame.dataY(), i420Frame.sizeOfLuminancePlane());
  memcpy(buffer->MutableDataU(), i420Frame.dataU(), i420Frame.sizeOfChromaPlane());
  memcpy(buffer->MutableDataV(), i420Frame.dataV(), i420Frame.sizeOfChromaPlane());
  return buffer;
}

CONVERTER_IMPL(node_webrtc::I420ImageData, rtc::scoped_refptr<webrtc::I420Buffer>, value) {
  return node_webrtc::Pure(CreateI420Buffer(value));
}

static node_webrtc::Validation<node_webrtc::RTCOnDataEventDict> CreateRTCOnDataEventDict(
    v8::ArrayBuffer::Contents samples,
    uint8_t bitsPerSample,
    uint16_t sampleRate,
    uint8_t channelCount,
    node_webrtc::Maybe<uint16_t> maybeNumberOfFrames
) {
  if (bitsPerSample != 16) {
    auto error = "Expected a .bitsPerSample of 16, not " + std::to_string(bitsPerSample);
    return node_webrtc::Validation<node_webrtc::RTCOnDataEventDict>::Invalid(error);
  }

  uint16_t numberOfFrames = sampleRate / 100;  // 10 ms
  auto actualNumberOfFrames = maybeNumberOfFrames.FromMaybe(numberOfFrames);
  if (actualNumberOfFrames != numberOfFrames) {
    auto error = "Expected a .numberOfFrames of " + std::to_string(numberOfFrames) + ", not " + std::to_string(actualNumberOfFrames);
    return node_webrtc::Validation<node_webrtc::RTCOnDataEventDict>::Invalid(error);
  }

  auto actualByteLength = samples.ByteLength();
  auto expectedByteLength = static_cast<size_t>(channelCount * numberOfFrames * bitsPerSample / 8);
  if (actualByteLength != expectedByteLength) {
    auto error = "Expected a .byteLength of " + std::to_string(expectedByteLength) + ", not " +
        std::to_string(actualByteLength);
    return node_webrtc::Validation<node_webrtc::RTCOnDataEventDict>::Invalid(error);
  }

  std::unique_ptr<uint8_t[]> samplesCopy(new uint8_t[actualByteLength]);
  if (!samplesCopy) {
    auto error = "Failed to copy samples";
    return node_webrtc::Validation<node_webrtc::RTCOnDataEventDict>::Invalid(error);
  }
  memcpy(samplesCopy.get(), samples.Data(), actualByteLength);

  node_webrtc::RTCOnDataEventDict dict = {
    samplesCopy.release(),
    bitsPerSample,
    sampleRate,
    channelCount,
    node_webrtc::MakeJust<uint16_t>(numberOfFrames)
  };

  return node_webrtc::Pure(dict);
}

FROM_JS_IMPL(node_webrtc::RTCOnDataEventDict, value) {
  return node_webrtc::From<v8::Local<v8::Object>>(value).FlatMap<node_webrtc::RTCOnDataEventDict>(
  [](const v8::Local<v8::Object> object) {
    return node_webrtc::Validation<node_webrtc::RTCOnDataEventDict>::Join(
            curry(CreateRTCOnDataEventDict)
            % node_webrtc::GetRequired<v8::ArrayBuffer::Contents>(object, "samples")
            * node_webrtc::GetOptional<uint8_t>(object, "bitsPerSample", 16)
            * node_webrtc::GetRequired<uint16_t>(object, "sampleRate")
            * node_webrtc::GetOptional<uint8_t>(object, "channelCount", 1)
            * node_webrtc::GetOptional<uint16_t>(object, "numberOfFrames")
        );
  });
}

#define REQUIRED(type, memberName, stringValue) EXPAND_OBJ_FROM_JS_REQUIRED(type, stringValue)
#define OPTIONAL(type, memberName, stringValue) EXPAND_OBJ_FROM_JS_OPTIONAL(type, stringValue)
#define DEFAULT(type, memberName, stringValue, defaultValue) EXPAND_OBJ_FROM_JS_DEFAULT(type, stringValue, defaultValue)
OBJ_FROM_JS_IMPL1(RTCOAUTHCREDENTIAL, CreateRTCOAuthCredential)
OBJ_FROM_JS_IMPL2(ICESERVER, CreateIceServer)
OBJ_FROM_JS_IMPL1(RTCDTLSFINGERPRINT, CreateRTCDtlsFingerprint)
OBJ_FROM_JS_IMPL2(UNSIGNEDSHORTRANGE, CreateUnsignedShortRange)
OBJ_FROM_JS_IMPL1(RTCOFFEROPTIONS, CreateRTCOfferOptions)
OBJ_FROM_JS_IMPL1(RTCANSWEROPTIONS, CreateRTCAnswerOptions)
OBJ_FROM_JS_IMPL1(RTCSESSIONDESCRIPTIONINIT, CreateRTCSessionDescriptionInit)
OBJ_FROM_JS_IMPL1(RTCVIDEOSOURCEINIT, CreateRTCVideoSourceInit)
OBJ_FROM_JS_IMPL2(ICECANDIDATEINTERFACE, CreateIceCandidateInterface)
OBJ_FROM_JS_IMPL2(DATACHANNELINIT, CreateDataChannelInit)
OBJ_FROM_JS_IMPL1(RTCRTPTRANSCEIVERINIT, CreateRtpTransceiverInit)
#undef REQUIRED
#undef OPTIONAL
#undef DEFAULT

TO_JS_IMPL(rtc::scoped_refptr<webrtc::VideoFrameBuffer>, value) {
  return value->type() == webrtc::VideoFrameBuffer::Type::kI420
      ? node_webrtc::From<v8::Local<v8::Value>>(value->GetI420())
      : node_webrtc::Validation<v8::Local<v8::Value>>::Invalid("Unsupported RTCVideoFrame type (file a node-webrtc bug, please!)");
}

TO_JS_IMPL(rtc::scoped_refptr<webrtc::I420BufferInterface>, value) {
  Nan::EscapableHandleScope scope;
  auto isolate = Nan::GetCurrentContext()->GetIsolate();

  auto sizeOfYPlane = value->StrideY() * value->height();
  auto sizeOfUPlane = value->StrideU() * value->height() / 2;
  auto sizeOfVPlane = value->StrideV() * value->height() / 2;

  auto byteLength = sizeOfYPlane + sizeOfUPlane + sizeOfVPlane;
  auto arrayBuffer = v8::ArrayBuffer::New(isolate, byteLength);
  uint8_t* data = static_cast<uint8_t*>(arrayBuffer->GetContents().Data());

  auto srcYPlane = value->DataY();
  auto srcUPlane = value->DataU();
  auto srcVPlane = value->DataV();

  auto dstYPlane = data;
  auto dstUPlane = data + sizeOfYPlane;
  auto dstVPlane = dstUPlane + sizeOfUPlane;

  memcpy(dstYPlane, srcYPlane, sizeOfYPlane);
  memcpy(dstUPlane, srcUPlane, sizeOfUPlane);
  memcpy(dstVPlane, srcVPlane, sizeOfVPlane);

  auto uint8Array = v8::Uint8ClampedArray::New(arrayBuffer, 0, byteLength);
  return node_webrtc::Pure(scope.Escape(uint8Array.As<v8::Value>()));
}

TO_JS_IMPL(webrtc::VideoFrame, value) {
  Nan::EscapableHandleScope scope;
  auto frame = Nan::New<v8::Object>();
  frame->Set(Nan::New("width").ToLocalChecked(), node_webrtc::From<v8::Local<v8::Value>>(value.width()).UnsafeFromValid());
  frame->Set(Nan::New("height").ToLocalChecked(), node_webrtc::From<v8::Local<v8::Value>>(value.height()).UnsafeFromValid());
  frame->Set(Nan::New("rotation").ToLocalChecked(), node_webrtc::From<v8::Local<v8::Value>>(static_cast<int>(value.rotation())).UnsafeFromValid());
  auto maybeData = node_webrtc::From<v8::Local<v8::Value>>(value.video_frame_buffer());
  if (maybeData.IsInvalid()) {
    return node_webrtc::Validation<v8::Local<v8::Value>>::Invalid(maybeData.ToErrors()[0]);
  }
  auto data = maybeData.UnsafeFromValid();
  frame->Set(Nan::New("data").ToLocalChecked(), data);
  return node_webrtc::Pure(scope.Escape(frame).As<v8::Value>());
}

TO_JS_IMPL(node_webrtc::RTCOnDataEventDict, dict) {
  Nan::EscapableHandleScope scope;

  if (dict.numberOfFrames.IsNothing()) {
    return node_webrtc::Validation<v8::Local<v8::Value>>::Invalid("numberOfFrames not provided");
  }
  auto numberOfFrames = dict.numberOfFrames.UnsafeFromJust();

  auto isolate = Nan::GetCurrentContext()->GetIsolate();
  auto length = dict.channelCount * numberOfFrames;
  auto byteLength = length * dict.bitsPerSample / 8;
  auto arrayBuffer = v8::ArrayBuffer::New(isolate, dict.samples, byteLength, v8::ArrayBufferCreationMode::kInternalized);

  v8::Local<v8::Value> samples;
  switch (dict.bitsPerSample) {
    case 8:
      samples = v8::Int8Array::New(arrayBuffer, 0, length);
      break;
    case 16:
      samples = v8::Int16Array::New(arrayBuffer, 0, length);
      break;
    case 32:
      samples = v8::Int32Array::New(arrayBuffer, 0, length);
      break;
    default:
      samples = v8::Uint8Array::New(arrayBuffer, 0, byteLength);
  }

  auto object = Nan::New<v8::Object>();
  object->Set(Nan::New("samples").ToLocalChecked(), samples);
  object->Set(Nan::New("bitsPerSample").ToLocalChecked(), node_webrtc::From<v8::Local<v8::Value>>(dict.bitsPerSample).UnsafeFromValid());
  object->Set(Nan::New("sampleRate").ToLocalChecked(), node_webrtc::From<v8::Local<v8::Value>>(dict.sampleRate).UnsafeFromValid());
  object->Set(Nan::New("channelCount").ToLocalChecked(), node_webrtc::From<v8::Local<v8::Value>>(dict.channelCount).UnsafeFromValid());
  object->Set(Nan::New("numberOfFrames").ToLocalChecked(), node_webrtc::From<v8::Local<v8::Value>>(numberOfFrames).UnsafeFromValid());
  return node_webrtc::Pure(scope.Escape(object).As<v8::Value>());
}
