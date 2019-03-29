/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/converters/dictionaries.h"

#include <nan.h>
#include <webrtc/api/data_channel_interface.h>
#include <webrtc/api/jsep.h>
#include <webrtc/api/peer_connection_interface.h>
#include <webrtc/api/rtc_error.h>
#include <webrtc/api/rtp_parameters.h>
#include <webrtc/api/rtp_receiver_interface.h>
#include <webrtc/api/rtp_transceiver_interface.h>
#include <webrtc/api/stats/rtc_stats.h>
#include <webrtc/api/video/i420_buffer.h>
#include <v8.h>

#include "src/converters.h"
#include "src/converters/interfaces.h"
#include "src/converters/object.h"
#include "src/converters/v8.h"
#include "src/functional/curry.h"
#include "src/functional/maybe.h"
#include "src/functional/operators.h"
#include "src/functional/either.h"
#include "src/i420helpers.h"
#include "src/errorfactory.h"
#include "src/mediastream.h"
#include "src/mediastreamtrack.h"
#include "src/rtcrtpreceiver.h"
#include "src/rtcrtpsender.h"
#include "src/rtcrtptransceiver.h"
#include "src/rtcstatsresponse.h"

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

#define DATACHANNELINIT webrtc::DataChannelInit
#define DATACHANNELINIT_LIST \
  DEFAULT(bool, ordered, "ordered", true) \
  OPTIONAL(uint32_t, maxPacketLifeTime, "maxPacketLifeTime") \
  OPTIONAL(uint32_t, maxRetransmits, "maxRetransmits") \
  DEFAULT(std::string, protocol, "protocol", "") \
  DEFAULT(bool, negotiated, "negotiated", false) \
  OPTIONAL(uint32_t, id, "id") \
  DEFAULT(node_webrtc::RTCPriorityType, priority, "priority", node_webrtc::RTCPriorityType::kLow)

#define ICECANDIDATEINTERFACE webrtc::IceCandidateInterface*
#define ICECANDIDATEINTERFACE_LIST \
  DEFAULT(std::string, candidate, "candidate", "") \
  DEFAULT(std::string, sdpMid, "sdpMid", "") \
  DEFAULT(int, sdpMLineIndex, "sdpMLineIndex", 0) \
  OPTIONAL(std::string, usernameFragment, "usernameFragment")

#define ICESERVER webrtc::PeerConnectionInterface::IceServer
#define ICESERVER_LIST \
  REQUIRED(stringOrStrings, urls, "urls") \
  DEFAULT(std::string, username, "username", "") \
  DEFAULT(stringOrCredential, credential, "credential", node_webrtc::MakeLeft<node_webrtc::RTCOAuthCredential>(std::string(""))) \
  DEFAULT(node_webrtc::RTCIceCredentialType, credentialType, "credentialType", node_webrtc::RTCIceCredentialType::kPassword)

#define RTCRTPTRANSCEIVERINIT webrtc::RtpTransceiverInit
#define RTCRTPTRANSCEIVERINIT_LIST \
  DEFAULT(webrtc::RtpTransceiverDirection, direction, "direction", webrtc::RtpTransceiverDirection::kSendRecv) \
  DEFAULT(std::vector<node_webrtc::MediaStream*>, streams, "streams", std::vector<node_webrtc::MediaStream*>())

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
    const webrtc::PeerConnectionInterface::RTCConfiguration& configuration,
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

TO_JS_IMPL(webrtc::IceCandidateInterface*, value) {
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

CONVERT_VIA(v8::Local<v8::Value>, webrtc::IceCandidateInterface*, std::shared_ptr<webrtc::IceCandidateInterface>)

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

namespace node_webrtc {

#define REQUIRED(type, memberName, stringValue) EXPAND_OBJ_FROM_JS_REQUIRED(type, stringValue)
#define OPTIONAL(type, memberName, stringValue) EXPAND_OBJ_FROM_JS_OPTIONAL(type, stringValue)
#define DEFAULT(type, memberName, stringValue, defaultValue) EXPAND_OBJ_FROM_JS_DEFAULT(type, stringValue, defaultValue)
OBJ_FROM_JS_IMPL2(ICESERVER, CreateIceServer)
OBJ_FROM_JS_IMPL2(ICECANDIDATEINTERFACE, CreateIceCandidateInterface)
OBJ_FROM_JS_IMPL2(DATACHANNELINIT, CreateDataChannelInit)
OBJ_FROM_JS_IMPL1(RTCRTPTRANSCEIVERINIT, CreateRtpTransceiverInit)
#undef REQUIRED
#undef OPTIONAL
#undef DEFAULT

}  // namespace node_webrtc

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
