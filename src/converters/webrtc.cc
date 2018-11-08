/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/converters/webrtc.h"

#include <nan.h>
#include <webrtc/api/datachannelinterface.h>
#include <webrtc/api/jsep.h>
#include <webrtc/api/mediastreaminterface.h>
#include <webrtc/api/peerconnectioninterface.h>
#include <webrtc/api/rtcerror.h>
#include <webrtc/api/rtpparameters.h>
#include <webrtc/api/rtpreceiverinterface.h>
#include <webrtc/api/rtptransceiverinterface.h>
#include <webrtc/api/stats/rtcstats.h>
#include <v8.h>

#include "src/asyncobjectwrapwithloop.h"  // IWYU pragma: keep
#include "src/converters.h"
#include "src/converters/v8.h"  // IWYU pragma: keep
#include "src/converters/object.h"
#include "src/functional/either.h"  // IWYU pragma: keep
#include "src/errorfactory.h"  // IWYU pragma: keep
#include "src/mediastream.h"  // IWYU pragma: keep
#include "src/mediastreamtrack.h"  // IWYU pragma: keep
#include "src/rtcrtpreceiver.h"  // IWYU pragma: keep
#include "src/rtcrtpsender.h"  // IWYU pragma: keep
#include "src/rtcrtptransceiver.h"  // IWYU pragma: keep
#include "src/rtcstatsresponse.h"  // IWYU pragma: keep

// IWYU pragma: no_include <api/mediatypes.h>

using Nan::EscapableHandleScope;
using node_webrtc::AsyncObjectWrapWithLoop;
using node_webrtc::BinaryType;
using node_webrtc::Converter;
using node_webrtc::Either;
using node_webrtc::ErrorFactory;
using node_webrtc::ExtendedRTCConfiguration;
using node_webrtc::From;
using node_webrtc::GetOptional;
using node_webrtc::GetRequired;
using node_webrtc::Maybe;
using node_webrtc::MediaStream;
using node_webrtc::MediaStreamTrack;
using node_webrtc::RTCAnswerOptions;
using node_webrtc::RTCDtlsFingerprint;
using node_webrtc::RTCIceCredentialType;
using node_webrtc::RTCOAuthCredential;
using node_webrtc::RTCOfferOptions;
using node_webrtc::RTCPeerConnectionState;
using node_webrtc::RTCPriorityType ;
using node_webrtc::RTCRtpReceiver;
using node_webrtc::RTCRtpSender;
using node_webrtc::RTCRtpTransceiver;
using node_webrtc::RTCSdpType;
using node_webrtc::RTCSessionDescriptionInit;
using node_webrtc::RTCStatsResponse;
using node_webrtc::RTCStatsResponseInit;
using node_webrtc::SomeError;
using node_webrtc::UnsignedShortRange;
using node_webrtc::Validation;
using v8::Array;
using v8::External;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::Value;
using webrtc::DataChannelInit;
using webrtc::IceCandidateInterface;
using webrtc::RTCError;
using webrtc::RTCErrorType;
using webrtc::SessionDescriptionInterface;

using BundlePolicy = webrtc::PeerConnectionInterface::BundlePolicy;
using DataState = webrtc::DataChannelInterface::DataState;
using IceConnectionState = webrtc::PeerConnectionInterface::IceConnectionState;
using IceGatheringState = webrtc::PeerConnectionInterface::IceGatheringState;
using IceServer = webrtc::PeerConnectionInterface::IceServer;
using IceTransportsType = webrtc::PeerConnectionInterface::IceTransportsType;
using RTCConfiguration = webrtc::PeerConnectionInterface::RTCConfiguration;
using RTCOfferAnswerOptions = webrtc::PeerConnectionInterface::RTCOfferAnswerOptions;
using RtcpMuxPolicy = webrtc::PeerConnectionInterface::RtcpMuxPolicy;
using SdpParseError = webrtc::SdpParseError;
using SignalingState = webrtc::PeerConnectionInterface::SignalingState ;

#define CONVERTER(I, O, V) Validation<O> Converter<I, O>::Convert(I V)
#define TO_JS(T, V) Validation<Local<Value>> Converter<T, Local<Value>>::Convert(T V)
#define FROM_JS(T, V) Validation<T> Converter<Local<Value>, T>::Convert(Local<Value> V)

#define FROM_JS_ENUM(T) \
  Validation<T> Converter<Local<Value>, T>::Convert(Local<Value> value) { \
    return From<std::string>(value).FlatMap<T>(Converter<std::string, T>::Convert); \
  }

#define TO_JS_ENUM(T) \
  Validation<Local<Value>> Converter<T, Local<Value>>::Convert(T value) { \
    return From<std::string>(value).FlatMap<Local<Value>>(Converter<std::string, Local<Value>>::Convert); \
  }

CONVERTER(cricket::MediaType, std::string, value) {
  switch (value) {
    case cricket::MediaType::MEDIA_TYPE_AUDIO:
      return Validation<std::string>::Valid("audio");
    case cricket::MediaType::MEDIA_TYPE_VIDEO:
      return Validation<std::string>::Valid("video");
    case cricket::MediaType::MEDIA_TYPE_DATA:
      return Validation<std::string>::Valid("data");
  }
}

CONVERTER(std::string, cricket::MediaType, value) {
  if (value == "audio") {
    return Validation<cricket::MediaType>::Valid(cricket::MediaType::MEDIA_TYPE_AUDIO);
  } else if (value == "video") {
    return Validation<cricket::MediaType>::Valid(cricket::MediaType::MEDIA_TYPE_VIDEO);
  } else if (value == "data") {
    return Validation<cricket::MediaType>::Valid(cricket::MediaType::MEDIA_TYPE_DATA);
  }
  return Validation<cricket::MediaType>::Invalid(R"(Expected "audio", "video" or "data")");
}

TO_JS_ENUM(cricket::MediaType)
FROM_JS_ENUM(cricket::MediaType)

static RTCOAuthCredential CreateRTCOAuthCredential(
    const std::string& macKey,
    const std::string& accessToken) {
  return RTCOAuthCredential(macKey, accessToken);
}

FROM_JS(RTCOAuthCredential, value) {
  return From<Local<Object>>(value).FlatMap<RTCOAuthCredential>(
  [](const Local<Object> object) {
    return curry(CreateRTCOAuthCredential)
        % GetRequired<std::string>(object, "macKey")
        * GetRequired<std::string>(object, "accessToken");
  });
}

FROM_JS(RTCIceCredentialType, value) {
  return From<std::string>(value).FlatMap<RTCIceCredentialType>(
  [](const std::string string) {
    if (string == "password") {
      return Validation<RTCIceCredentialType>::Valid(kPassword);
    } else if (string == "oauth") {
      return Validation<RTCIceCredentialType>::Valid(kOAuth);
    }
    return Validation<RTCIceCredentialType>::Invalid(R"(Expected "password" or "oauth")");
  });
}

static Validation<IceServer> CreateIceServer(
    const Either<std::vector<std::string>, std::string>& urlsOrUrl,
    const std::string& username,
    const Either<std::string, RTCOAuthCredential>& credential,
    const RTCIceCredentialType credentialType) {
  if (credential.IsRight() || credentialType != RTCIceCredentialType::kPassword) {
    return Validation<IceServer>::Invalid("OAuth is not currently supported");
  }
  IceServer iceServer;
  iceServer.urls = urlsOrUrl.FromLeft(std::vector<std::string>());
  iceServer.uri = urlsOrUrl.FromRight("");
  iceServer.username = username;
  iceServer.password = credential.UnsafeFromLeft();
  return Validation<IceServer>::Valid(iceServer);
}

FROM_JS(IceServer, value) {
  return From<Local<Object>>(value).FlatMap<IceServer>(
  [](const Local<Object> object) {
    return Validation<IceServer>::Join(curry(CreateIceServer)
            % GetRequired<Either<std::vector<std::string>, std::string>>(object, "urls")
            * GetOptional<std::string>(object, "username", "")
            * GetOptional<Either<std::string, RTCOAuthCredential>>(object, "credential", Either<std::string, RTCOAuthCredential>::Left(""))
            * GetOptional<RTCIceCredentialType>(object, "credentialType", kPassword));
  });
}

TO_JS(IceServer, iceServer) {
  EscapableHandleScope scope;
  auto object = Nan::New<Object>();
  if (!iceServer.uri.empty()) {
    object->Set(Nan::New("urls").ToLocalChecked(), Nan::New(iceServer.uri).ToLocalChecked());
  } else {
    auto maybeArray = From<Local<Value>>(iceServer.urls);
    if (maybeArray.IsInvalid()) {
      return Validation<Local<Value>>::Invalid(maybeArray.ToErrors());
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
  return Validation<Local<Value>>::Valid(scope.Escape(object));
}

CONVERTER(std::string, IceTransportsType, value) {
  if (value == "all") {
    return Validation<IceTransportsType >::Valid(IceTransportsType::kAll);
  } else if (value == "relay") {
    return Validation<IceTransportsType >::Valid(IceTransportsType::kRelay);
  }
  return Validation<IceTransportsType>::Invalid(R"(Expected "all" or "relay")");
}

CONVERTER(IceTransportsType, std::string, value) {
  switch (value) {
    case IceTransportsType::kAll:
      return Validation<std::string>::Valid("all");
    case IceTransportsType::kRelay:
      return Validation<std::string>::Valid("relay");
    case IceTransportsType::kNoHost:
    case IceTransportsType::kNone:
      return Validation<std::string>::Invalid(
              "Somehow you've set RTCIceTransportPolicy to an unsupported value; "
              "please file a bug at https://github.com/js-platform/node-webrtc");
  }
}

FROM_JS_ENUM(IceTransportsType)
TO_JS_ENUM(IceTransportsType)

FROM_JS(BundlePolicy, value) {
  return From<std::string>(value).FlatMap<BundlePolicy>(
  [](const std::string string) {
    if (string == "balanced") {
      return Validation<BundlePolicy>::Valid(BundlePolicy::kBundlePolicyBalanced);
    } else if (string == "max-compat") {
      return Validation<BundlePolicy>::Valid(BundlePolicy::kBundlePolicyMaxCompat);
    } else if (string == "max-bundle") {
      return Validation<BundlePolicy>::Valid(BundlePolicy::kBundlePolicyMaxBundle);
    }
    return Validation<BundlePolicy>::Invalid(R"(Expected "balanced", "max-compat" or "max-bundle")");
  });
}

TO_JS(BundlePolicy, type) {
  EscapableHandleScope scope;
  switch (type) {
    case BundlePolicy::kBundlePolicyBalanced:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("balanced").ToLocalChecked()));
    case BundlePolicy::kBundlePolicyMaxBundle:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("max-bundle").ToLocalChecked()));
    case BundlePolicy::kBundlePolicyMaxCompat:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("max-compat").ToLocalChecked()));
  }
}

FROM_JS(RtcpMuxPolicy, value) {
  return From<std::string>(value).FlatMap<RtcpMuxPolicy>(
  [](const std::string string) {
    if (string == "negotiate") {
      return Validation<RtcpMuxPolicy>::Valid(RtcpMuxPolicy::kRtcpMuxPolicyNegotiate);
    } else if (string == "require") {
      return Validation<RtcpMuxPolicy>::Valid(RtcpMuxPolicy::kRtcpMuxPolicyRequire);
    }
    return Validation<RtcpMuxPolicy>::Invalid(R"(Expected "negotiate" or "require")");
  });
}

TO_JS(RtcpMuxPolicy, type) {
  EscapableHandleScope scope;
  switch (type) {
    case RtcpMuxPolicy::kRtcpMuxPolicyNegotiate:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("negotiate").ToLocalChecked()));
    case RtcpMuxPolicy::kRtcpMuxPolicyRequire:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("require").ToLocalChecked()));
  }
}

static RTCDtlsFingerprint CreateRTCDtlsFingerprint(
    const Maybe<std::string>& algorithm,
    const Maybe<std::string>& value) {
  return RTCDtlsFingerprint(algorithm, value);
}

FROM_JS(RTCDtlsFingerprint, value) {
  return From<Local<Object>>(value).FlatMap<RTCDtlsFingerprint>(
  [](const Local<Object> object) {
    return curry(CreateRTCDtlsFingerprint)
        % GetOptional<std::string>(object, "algorithm")
        * GetOptional<std::string>(object, "value");
  });
}

static Validation<UnsignedShortRange> CreateUnsignedShortRange(
    const Maybe<uint16_t>& maybeMin,
    const Maybe<uint16_t>& maybeMax) {
  auto min = maybeMin.FromMaybe(0);
  auto max = maybeMax.FromMaybe(65535);
  if (min > max) {
    return Validation<UnsignedShortRange>::Invalid("Expected min to be less than max");
  }
  return Validation<UnsignedShortRange>::Valid(UnsignedShortRange(maybeMin, maybeMax));
}

FROM_JS(UnsignedShortRange, value) {
  return From<Local<Object>>(value).FlatMap<UnsignedShortRange>(
  [](const Local<Object> object) {
    return Validation<UnsignedShortRange>::Join(curry(CreateUnsignedShortRange)
            % GetOptional<uint16_t>(object, "min")
            * GetOptional<uint16_t>(object, "max"));
  });
}

TO_JS(UnsignedShortRange, value) {
  EscapableHandleScope scope;
  auto object = Nan::New<Object>();
  if (value.min.IsJust()) {
    object->Set(Nan::New("min").ToLocalChecked(), Nan::New(value.min.UnsafeFromJust()));
  }
  if (value.max.IsJust()) {
    object->Set(Nan::New("max").ToLocalChecked(), Nan::New(value.max.UnsafeFromJust()));
  }
  return Validation<Local<Value>>::Valid(scope.Escape(object));
}

CONVERTER(webrtc::SdpSemantics, std::string, value) {
  switch (value) {
    case webrtc::SdpSemantics::kPlanB:
      return Validation<std::string>::Valid("plan-b");
    case webrtc::SdpSemantics::kUnifiedPlan:
      return Validation<std::string>::Valid("unified-plan");
  }
}

CONVERTER(std::string, webrtc::SdpSemantics, value) {
  if (value == "plan-b") {
    return Validation<webrtc::SdpSemantics>::Valid(webrtc::SdpSemantics::kPlanB);
  } else if (value == "unified-plan") {
    return Validation<webrtc::SdpSemantics>::Valid(webrtc::SdpSemantics::kUnifiedPlan);
  }
  return Validation<webrtc::SdpSemantics>::Invalid(R"(Expected "plan-b" or "unified-plan")");
}

TO_JS_ENUM(webrtc::SdpSemantics)
FROM_JS_ENUM(webrtc::SdpSemantics)

static RTCConfiguration CreateRTCConfiguration(
    const std::vector<IceServer>& iceServers,
    const IceTransportsType iceTransportsPolicy,
    const BundlePolicy bundlePolicy,
    const RtcpMuxPolicy rtcpMuxPolicy,
    const Maybe<std::string>&,
    const Maybe<std::vector<Local<Object>>>&,
    const uint32_t iceCandidatePoolSize,
    const webrtc::SdpSemantics sdpSemantics) {
  RTCConfiguration configuration;
  configuration.servers = iceServers;
  configuration.type = iceTransportsPolicy;
  configuration.bundle_policy = bundlePolicy;
  configuration.rtcp_mux_policy = rtcpMuxPolicy;
  configuration.ice_candidate_pool_size = iceCandidatePoolSize;
  configuration.sdp_semantics = sdpSemantics;
  return configuration;
}

FROM_JS(RTCConfiguration, value) {
  // NOTE(mroberts): Allow overriding the default SdpSemantics via environment variable.
  // Makes web-platform-tests easier to run.
  auto sdp_semantics_env = std::getenv("SDP_SEMANTICS");
  auto sdp_semantics_str = sdp_semantics_env ? std::string(sdp_semantics_env) : std::string();
  auto sdp_semantics = From<webrtc::SdpSemantics>(sdp_semantics_str).FromValidation(webrtc::SdpSemantics::kPlanB);
  return From<Local<Object>>(value).FlatMap<RTCConfiguration>(
  [sdp_semantics](const Local<Object> object) {
    return curry(CreateRTCConfiguration)
        % GetOptional<std::vector<IceServer>>(object, "iceServers", std::vector<IceServer>())
        * GetOptional<IceTransportsType>(object, "iceTransportPolicy", IceTransportsType::kAll)
        * GetOptional<BundlePolicy>(object, "bundlePolicy", BundlePolicy::kBundlePolicyBalanced)
        * GetOptional<RtcpMuxPolicy>(object, "rtcpMuxPolicy", RtcpMuxPolicy::kRtcpMuxPolicyRequire)
        * GetOptional<std::string>(object, "peerIdentity")
        * GetOptional<std::vector<Local<Object>>>(object, "certificates")
        // TODO(mroberts): Implement EnforceRange and change to uint8_t.
        * GetOptional<uint8_t>(object, "iceCandidatePoolSize", 0)
        * GetOptional<webrtc::SdpSemantics>(object, "sdpSemantics", sdp_semantics);
  });
}

static ExtendedRTCConfiguration CreateExtendedRTCConfiguration(
    const RTCConfiguration configuration,
    const UnsignedShortRange portRange) {
  return ExtendedRTCConfiguration(configuration, portRange);
}

FROM_JS(ExtendedRTCConfiguration, value) {
  return From<Local<Object>>(value).FlatMap<ExtendedRTCConfiguration>(
  [](const Local<Object> object) {
    return curry(CreateExtendedRTCConfiguration)
        % From<RTCConfiguration>(static_cast<Local<Value>>(object))
        * GetOptional<UnsignedShortRange>(object, "portRange", UnsignedShortRange());
  });
}

static Local<Value> ExtendedRTCConfigurationToJavaScript(
    const Local<Value> iceServers,
    const Local<Value> iceTransportPolicy,
    const Local<Value> bundlePolicy,
    const Local<Value> rtcpMuxPolicy,
    const Local<Value> iceCandidatePoolSize,
    const Local<Value> portRange,
    const Local<Value> sdpSemantics) {
  EscapableHandleScope scope;
  auto object = Nan::New<Object>();
  object->Set(Nan::New("iceServers").ToLocalChecked(), iceServers);
  object->Set(Nan::New("iceTransportPolicy").ToLocalChecked(), iceTransportPolicy);
  object->Set(Nan::New("bundlePolicy").ToLocalChecked(), bundlePolicy);
  object->Set(Nan::New("rtcpMuxPolicy").ToLocalChecked(), rtcpMuxPolicy);
  object->Set(Nan::New("iceCandidatePoolSize").ToLocalChecked(), iceCandidatePoolSize);
  object->Set(Nan::New("portRange").ToLocalChecked(), portRange);
  object->Set(Nan::New("sdpSemantics").ToLocalChecked(), sdpSemantics);
  return scope.Escape(object);
}

TO_JS(ExtendedRTCConfiguration, configuration) {
  return curry(ExtendedRTCConfigurationToJavaScript)
      % From<Local<Value>>(configuration.configuration.servers)
      * From<Local<Value>>(configuration.configuration.type)
      * From<Local<Value>>(configuration.configuration.bundle_policy)
      * From<Local<Value>>(configuration.configuration.rtcp_mux_policy)
      * Validation<Local<Value>>::Valid(Nan::New(configuration.configuration.ice_candidate_pool_size))
      * From<Local<Value>>(configuration.portRange)
      * From<Local<Value>>(configuration.configuration.sdp_semantics);
}

static RTCOfferOptions CreateRTCOfferOptions(
    const bool voiceActivityDetection,
    const bool iceRestart,
    const Maybe<bool> offerToReceiveAudio,
    const Maybe<bool> offerToReceiveVideo) {
  RTCOfferAnswerOptions options;
  options.ice_restart = iceRestart;
  options.voice_activity_detection = voiceActivityDetection;
  options.offer_to_receive_audio = offerToReceiveAudio.Map(
  [](const bool boolean) { return boolean ? RTCOfferAnswerOptions::kOfferToReceiveMediaTrue : 0; }
      ).FromMaybe(RTCOfferAnswerOptions::kUndefined);
  options.offer_to_receive_video = offerToReceiveVideo.Map(
  [](const bool boolean) { return boolean ? RTCOfferAnswerOptions::kOfferToReceiveMediaTrue : 0; }
      ).FromMaybe(RTCOfferAnswerOptions::kUndefined);
  return RTCOfferOptions(options);
}

FROM_JS(RTCOfferOptions, value) {
  return From<Local<Object>>(value).FlatMap<RTCOfferOptions>(
  [](const Local<Object> object) {
    return curry(CreateRTCOfferOptions)
        % GetOptional<bool>(object, "voiceActivityDetection", true)
        * GetOptional<bool>(object, "iceRestart", false)
        * GetOptional<bool>(object, "offerToReceiveAudio")
        * GetOptional<bool>(object, "offerToReceiveVideo");
  });
}

static RTCAnswerOptions CreateRTCAnswerOptions(const bool voiceActivityDetection) {
  RTCOfferAnswerOptions options;
  options.voice_activity_detection = voiceActivityDetection;
  return RTCAnswerOptions(options);
}

FROM_JS(RTCAnswerOptions, value) {
  return From<Local<Object>>(value).FlatMap<RTCAnswerOptions>(
  [](const Local<Object> object) {
    return curry(CreateRTCAnswerOptions)
        % GetOptional<bool>(object, "voiceActivityDetection", true);
  });
}

Validation<RTCSdpType> Converter<std::string, RTCSdpType>::Convert(const std::string value) {
  if (value == "offer") {
    return Validation<RTCSdpType>::Valid(RTCSdpType::kOffer);
  } else if (value == "pranswer") {
    return Validation<RTCSdpType>::Valid(RTCSdpType::kPrAnswer);
  } else if (value == "answer") {
    return Validation<RTCSdpType>::Valid(RTCSdpType::kAnswer);
  } else if (value == "rollback") {
    return Validation<RTCSdpType>::Valid(RTCSdpType::kRollback);
  }
  return Validation<RTCSdpType>::Invalid(R"(Expected "offer", "pranswer", "answer" or "rollback")");
}

Validation<std::string> Converter<RTCSdpType, std::string>::Convert(const RTCSdpType type) {
  switch (type) {
    case RTCSdpType::kOffer:
      return Validation<std::string>::Valid("offer");
    case RTCSdpType::kAnswer:
      return Validation<std::string>::Valid("answer");
    case RTCSdpType::kPrAnswer:
      return Validation<std::string>::Valid("pranswer");
    case RTCSdpType::kRollback:
      return Validation<std::string>::Valid("rollback");
  }
}

FROM_JS_ENUM(RTCSdpType)

static RTCSessionDescriptionInit CreateRTCSessionDescriptionInit(
    const RTCSdpType type,
    const std::string sdp) {
  return RTCSessionDescriptionInit(type, sdp);
}

FROM_JS(RTCSessionDescriptionInit, value) {
  return From<Local<Object>>(value).FlatMap<RTCSessionDescriptionInit>(
  [](const Local<Object> object) {
    return curry(CreateRTCSessionDescriptionInit)
        % GetRequired<RTCSdpType>(object, "type")
        * GetOptional<std::string>(object, "sdp", "");
  });
}

Validation<SessionDescriptionInterface*> Converter<RTCSessionDescriptionInit, SessionDescriptionInterface*>::Convert(
    const RTCSessionDescriptionInit init) {
  std::string type_;
  switch (init.type) {
    case RTCSdpType::kOffer:
      type_ = "offer";
      break;
    case RTCSdpType::kPrAnswer:
      type_ = "pranswer";
      break;
    case RTCSdpType::kAnswer:
      type_ = "answer";
      break;
    case RTCSdpType::kRollback:
      return Validation<SessionDescriptionInterface*>::Invalid("Rollback is not currently supported");
  }
  SdpParseError error;
  auto description = webrtc::CreateSessionDescription(type_, init.sdp, &error);
  if (!description) {
    return Validation<SessionDescriptionInterface*>::Invalid(error.description);
  }
  return Validation<SessionDescriptionInterface*>::Valid(description);
}

TO_JS(RTCSessionDescriptionInit, init) {
  EscapableHandleScope scope;
  auto maybeType = From<std::string>(init.type);
  if (maybeType.IsInvalid()) {
    return Validation<Local<Value>>::Invalid(maybeType.ToErrors()[0]);
  }
  auto object = Nan::New<Object>();
  object->Set(Nan::New("sdp").ToLocalChecked(), Nan::New(init.sdp).ToLocalChecked());
  object->Set(Nan::New("type").ToLocalChecked(), Nan::New(maybeType.UnsafeFromValid()).ToLocalChecked());
  return Validation<Local<Value>>::Valid(scope.Escape(object));
}

Validation<RTCSessionDescriptionInit> Converter<SessionDescriptionInterface*, RTCSessionDescriptionInit>::Convert(
    SessionDescriptionInterface* description) {
  if (!description) {
    return Validation<RTCSessionDescriptionInit>::Invalid("RTCSessionDescription is null");
  }
  std::string sdp;
  if (!description->ToString(&sdp)) {
    return Validation<RTCSessionDescriptionInit>::Invalid("Failed to print the SDP. This is pretty weird. File a bug on https://github.com/js-platform/node-webrtc");
  }
  return curry(CreateRTCSessionDescriptionInit)
      % From<RTCSdpType>(description->type())
      * Validation<std::string>(sdp);
}

FROM_JS(SessionDescriptionInterface*, value) {
  return From<RTCSessionDescriptionInit>(value)
      .FlatMap<SessionDescriptionInterface*>(Converter<RTCSessionDescriptionInit, SessionDescriptionInterface*>::Convert);
}

TO_JS(const SessionDescriptionInterface*, value) {
  EscapableHandleScope scope;

  if (!value) {
    return Validation<Local<Value>>::Invalid("RTCSessionDescription is null");
  }

  std::string sdp;
  if (!value->ToString(&sdp)) {
    return Validation<Local<Value>>::Invalid("Failed to print the SDP. This is pretty weird. File a bug on https://github.com/js-platform/node-webrtc");
  }

  auto object = Nan::New<Object>();
  object->Set(Nan::New("sdp").ToLocalChecked(), Nan::New(sdp).ToLocalChecked());
  object->Set(Nan::New("type").ToLocalChecked(), Nan::New(value->type()).ToLocalChecked());

  return Validation<Local<Value>>::Valid(scope.Escape(object));
}

static Validation<IceCandidateInterface*> CreateIceCandidateInterface(
    const std::string& candidate,
    const std::string& sdpMid,
    const int sdpMLineIndex,
    const Maybe<std::string>&) {
  SdpParseError error;
  auto candidate_ = webrtc::CreateIceCandidate(sdpMid, sdpMLineIndex, candidate, &error);
  if (!candidate_) {
    return Validation<IceCandidateInterface*>::Invalid(error.description);
  }
  return Validation<IceCandidateInterface*>::Valid(candidate_);
}

FROM_JS(IceCandidateInterface*, value) {
  return From<Local<Object>>(value).FlatMap<IceCandidateInterface*>(
  [](const Local<Object> object) {
    return Validation<IceCandidateInterface*>::Join(curry(CreateIceCandidateInterface)
            % GetOptional<std::string>(object, "candidate", "")
            * GetOptional<std::string>(object, "sdpMid", "")
            * GetOptional<int>(object, "sdpMLineIndex", 0)
            * GetOptional<std::string>(object, "usernameFragment"));
  });
}

TO_JS(const IceCandidateInterface*, value) {
  EscapableHandleScope scope;

  if (!value) {
    return Validation<Local<Value>>::Invalid("RTCIceCandidate is null");
  }

  std::string candidate;
  if (!value->ToString(&candidate)) {
    return Validation<Local<Value>>::Invalid("Failed to print the candidate string. This is pretty weird. File a bug on https://github.com/js-platform/node-webrtc");
  }

  auto object = Nan::New<Object>();
  object->Set(Nan::New("candidate").ToLocalChecked(), Nan::New(candidate).ToLocalChecked());
  object->Set(Nan::New("sdpMid").ToLocalChecked(), Nan::New(value->sdp_mid()).ToLocalChecked());
  object->Set(Nan::New("sdpMLineIndex").ToLocalChecked(), Nan::New(value->sdp_mline_index()));

  return Validation<Local<Value>>::Valid(scope.Escape(object));
}

FROM_JS(RTCPriorityType, value) {
  return From<std::string>(value).FlatMap<RTCPriorityType>(
  [](const std::string string) {
    if (string == "very-low") {
      return Validation<RTCPriorityType>::Valid(RTCPriorityType::kVeryLow);
    } else if (string == "low") {
      return Validation<RTCPriorityType>::Valid(RTCPriorityType::kLow);
    } else if (string == "medium") {
      return Validation<RTCPriorityType>::Valid(RTCPriorityType::kMedium);
    } else if (string == "high") {
      return Validation<RTCPriorityType>::Valid(RTCPriorityType::kHigh);
    }
    return Validation<RTCPriorityType>::Invalid(R"(Expected "very-low", "low", "medium" or "high")");
  });
}

static Validation<DataChannelInit> CreateDataChannelInit(
    const bool ordered,
    const Maybe<uint32_t> maxPacketLifeTime,
    const Maybe<uint32_t> maxRetransmits,
    const std::string& protocol,
    const bool negotiated,
    const Maybe<uint32_t> id,
    const RTCPriorityType) {
  if (id.FromMaybe(0) > UINT16_MAX) {
    return Validation<DataChannelInit>::Invalid("id must be between 0 and 65534, inclusive");
  } else if (maxPacketLifeTime.IsJust() && maxRetransmits.IsJust()) {
    return Validation<DataChannelInit>::Invalid("You cannot set both maxPacketLifeTime and maxRetransmits");
  }
  DataChannelInit init;
  init.ordered = ordered;
  init.maxRetransmitTime = maxPacketLifeTime.Map([](const uint32_t i) { return static_cast<int>(i); }).FromMaybe(-1);
  init.maxRetransmits = maxRetransmits.Map([](const uint32_t i) { return static_cast<int>(i); }).FromMaybe(-1);
  init.protocol = protocol;
  init.negotiated = negotiated;
  init.id = id.Map([](const uint32_t i) { return static_cast<int>(i); }).FromMaybe(-1);
  return Validation<DataChannelInit>::Valid(init);
}

FROM_JS(DataChannelInit, value) {
  return From<Local<Object>>(value).FlatMap<DataChannelInit>(
  [](const Local<Object> object) {
    return Validation<DataChannelInit>::Join(curry(CreateDataChannelInit)
            % GetOptional<bool>(object, "ordered", true)
            * GetOptional<uint32_t>(object, "maxPacketLifeTime")
            * GetOptional<uint32_t>(object, "maxRetransmits")
            * GetOptional<std::string>(object, "protocol", "")
            * GetOptional<bool>(object, "negotiated", false)
            * GetOptional<uint32_t>(object, "id")
            * GetOptional<RTCPriorityType>(object, "priority", kLow));
  });
}

TO_JS(SignalingState, state) {
  EscapableHandleScope scope;
  switch (state) {
    case SignalingState::kStable:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("stable").ToLocalChecked()));
    case SignalingState::kHaveLocalOffer:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("have-local-offer").ToLocalChecked()));
    case SignalingState::kHaveRemoteOffer:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("have-remote-offer").ToLocalChecked()));
    case SignalingState::kHaveLocalPrAnswer:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("have-local-pranswer").ToLocalChecked()));
    case SignalingState::kHaveRemotePrAnswer:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("have-remote-pranswer").ToLocalChecked()));
    case SignalingState::kClosed:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("closed").ToLocalChecked()));
  }
}

TO_JS(IceGatheringState, state) {
  EscapableHandleScope scope;
  switch (state) {
    case IceGatheringState::kIceGatheringNew:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("new").ToLocalChecked()));
    case IceGatheringState::kIceGatheringGathering:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("gathering").ToLocalChecked()));
    case IceGatheringState::kIceGatheringComplete:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("complete").ToLocalChecked()));
  }
}

TO_JS(IceConnectionState, state) {
  EscapableHandleScope scope;
  switch (state) {
    case IceConnectionState::kIceConnectionChecking:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("checking").ToLocalChecked()));
    case IceConnectionState::kIceConnectionClosed:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("closed").ToLocalChecked()));
    case IceConnectionState::kIceConnectionCompleted:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("completed").ToLocalChecked()));
    case IceConnectionState::kIceConnectionConnected:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("connected").ToLocalChecked()));
    case IceConnectionState::kIceConnectionDisconnected:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("disconnected").ToLocalChecked()));
    case IceConnectionState::kIceConnectionFailed:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("failed").ToLocalChecked()));
    case IceConnectionState::kIceConnectionMax:
      return Validation<Local<Value>>::Invalid(
              "WebRTC\'s RTCPeerConnection has an ICE connection state \"max\", but I have no idea"
              "what this means. If you see this error, file a bug on https://github.com/js-platform/node-webrtc");
    case IceConnectionState::kIceConnectionNew:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("new").ToLocalChecked()));
  }
}

TO_JS(DataState, state) {
  EscapableHandleScope scope;
  switch (state) {
    case DataState::kClosed:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("closed").ToLocalChecked()));
    case DataState::kClosing:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("closing").ToLocalChecked()));
    case DataState::kConnecting:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("connecting").ToLocalChecked()));
    case DataState::kOpen:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("open").ToLocalChecked()));
  }
}

TO_JS(BinaryType, binaryType) {
  EscapableHandleScope scope;
  switch (binaryType) {
    case BinaryType::kArrayBuffer:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("arraybuffer").ToLocalChecked()));
    case BinaryType::kBlob:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("blob").ToLocalChecked()));
  }
}

FROM_JS(BinaryType, value) {
  return From<std::string>(value).FlatMap<BinaryType>(
  [](const std::string string) {
    if (string == "blob") {
      return Validation<BinaryType>::Invalid(R"("blob" is not supported at this time; file a bug on https://github.com/js-platform/node-webrtc)");
    } else if (string == "arraybuffer") {
      return Validation<BinaryType>::Valid(BinaryType::kArrayBuffer);
    } else {
      return Validation<BinaryType>::Invalid(R"(Expected "blob" or "arraybuffer")");
    }
  });
}

TO_JS(RTCError*, error) {
  return Converter<const RTCError*, Local<Value>>::Convert(error);
}

TO_JS(const RTCError*, error) {
  EscapableHandleScope scope;
  if (!error) {
    return Validation<Local<Value>>::Invalid("No error? Please file a bug on https://github.com/js-platform/node-webrtc");
  }
  switch (error->type()) {
    case RTCErrorType::NONE:
      return Validation<Local<Value>>::Invalid("No error? Please file a bug on https://github.com/js-platform/node-webrtc");
    case RTCErrorType::UNSUPPORTED_PARAMETER:
    case RTCErrorType::INVALID_PARAMETER:
      return Validation<Local<Value>>::Valid(scope.Escape(ErrorFactory::CreateInvalidAccessError(error->message())));
    case RTCErrorType::INVALID_RANGE:
      return Validation<Local<Value>>::Valid(scope.Escape(ErrorFactory::CreateRangeError(error->message())));
    case RTCErrorType::SYNTAX_ERROR:
      return Validation<Local<Value>>::Valid(scope.Escape(ErrorFactory::CreateSyntaxError(error->message())));
    case RTCErrorType::INVALID_STATE:
      return Validation<Local<Value>>::Valid(scope.Escape(ErrorFactory::CreateInvalidStateError(error->message())));
    case RTCErrorType::INVALID_MODIFICATION:
      return Validation<Local<Value>>::Valid(scope.Escape(ErrorFactory::CreateInvalidModificationError(error->message())));
    case RTCErrorType::NETWORK_ERROR:
      return Validation<Local<Value>>::Valid(scope.Escape(ErrorFactory::CreateNetworkError(error->message())));
    // NOTE(mroberts): SetLocalDescription in the wrong state can throw this.
    case RTCErrorType::INTERNAL_ERROR:
      return Validation<Local<Value>>::Valid(scope.Escape(ErrorFactory::CreateInvalidStateError(error->message())));
    case RTCErrorType::UNSUPPORTED_OPERATION:
    case RTCErrorType::RESOURCE_EXHAUSTED:
      return Validation<Local<Value>>::Valid(scope.Escape(ErrorFactory::CreateOperationError(error->message())));
  }
}

Validation<SomeError> Converter<RTCError*, SomeError>::Convert(RTCError* error) {
  return Converter<const RTCError*, SomeError>::Convert(error);
}

Validation<SomeError> Converter<const RTCError*, SomeError>::Convert(const RTCError* error) {
  if (!error) {
    return Validation<SomeError>::Invalid("No error? Please file a bug on https://github.com/js-platform/node-webrtc");
  }
  auto type = Either<ErrorFactory::DOMExceptionName, ErrorFactory::ErrorName>::Right(ErrorFactory::ErrorName::kError);
  switch (error->type()) {
    case RTCErrorType::NONE:
      return Validation<SomeError>::Invalid("No error? Please file a bug on https://github.com/js-platform/node-webrtc");
    case RTCErrorType::UNSUPPORTED_PARAMETER:
    case RTCErrorType::INVALID_PARAMETER:
      type = Either<ErrorFactory::DOMExceptionName, ErrorFactory::ErrorName>::Left(ErrorFactory::DOMExceptionName::kInvalidAccessError);
      break;
    case RTCErrorType::INVALID_RANGE:
      type = Either<ErrorFactory::DOMExceptionName, ErrorFactory::ErrorName>::Right(ErrorFactory::ErrorName::kRangeError);
      break;
    case RTCErrorType::SYNTAX_ERROR:
      type = Either<ErrorFactory::DOMExceptionName, ErrorFactory::ErrorName>::Right(ErrorFactory::ErrorName::kSyntaxError);
      break;
    case RTCErrorType::INVALID_STATE:
      type = Either<ErrorFactory::DOMExceptionName, ErrorFactory::ErrorName>::Left(ErrorFactory::DOMExceptionName::kInvalidStateError);
      break;
    case RTCErrorType::INVALID_MODIFICATION:
      type = Either<ErrorFactory::DOMExceptionName, ErrorFactory::ErrorName>::Left(ErrorFactory::DOMExceptionName::kInvalidModificationError);
      break;
    case RTCErrorType::NETWORK_ERROR:
      type = Either<ErrorFactory::DOMExceptionName, ErrorFactory::ErrorName>::Left(ErrorFactory::DOMExceptionName::kNetworkError);
      break;
    // NOTE(mroberts): SetLocalDescription in the wrong state can throw this.
    case RTCErrorType::INTERNAL_ERROR:
      type = Either<ErrorFactory::DOMExceptionName, ErrorFactory::ErrorName>::Left(ErrorFactory::DOMExceptionName::kInvalidStateError);
      break;
    case RTCErrorType::UNSUPPORTED_OPERATION:
    case RTCErrorType::RESOURCE_EXHAUSTED:
      type = Either<ErrorFactory::DOMExceptionName, ErrorFactory::ErrorName>::Left(ErrorFactory::DOMExceptionName::kOperationError);
      break;
  }
  return Validation<SomeError>::Valid(SomeError(error->message(), type));
}

Validation<RTCPeerConnectionState> Converter<IceConnectionState, RTCPeerConnectionState>::Convert(IceConnectionState state) {
  switch (state) {
    case IceConnectionState::kIceConnectionNew:
      return Validation<RTCPeerConnectionState>::Valid(kNew);
    case IceConnectionState::kIceConnectionChecking:
      return Validation<RTCPeerConnectionState>::Valid(kConnecting);
    case IceConnectionState::kIceConnectionConnected:
    case IceConnectionState::kIceConnectionCompleted:
      return Validation<RTCPeerConnectionState>::Valid(kConnected);
    case IceConnectionState::kIceConnectionDisconnected:
      return Validation<RTCPeerConnectionState>::Valid(kDisconnected);
    case IceConnectionState::kIceConnectionFailed:
      return Validation<RTCPeerConnectionState>::Valid(kFailed);
    case IceConnectionState::kIceConnectionClosed:
      return Validation<RTCPeerConnectionState>::Valid(kClosed);
    case IceConnectionState::kIceConnectionMax:
      return Validation<RTCPeerConnectionState>::Invalid(
              "WebRTC\'s RTCPeerConnection has an ICE connection state \"max\", but I have no idea"
              "what this means. If you see this error, file a bug on https://github.com/js-platform/node-webrtc");
  }
}

TO_JS(RTCPeerConnectionState, state) {
  EscapableHandleScope scope;
  switch (state) {
    case RTCPeerConnectionState::kClosed:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("closed").ToLocalChecked()));
    case RTCPeerConnectionState::kConnected:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("connected").ToLocalChecked()));
    case RTCPeerConnectionState::kConnecting:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("connecting").ToLocalChecked()));
    case RTCPeerConnectionState::kDisconnected:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("disconnected").ToLocalChecked()));
    case RTCPeerConnectionState::kFailed:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("failed").ToLocalChecked()));
    case RTCPeerConnectionState::kNew:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("new").ToLocalChecked()));
  }
}

TO_JS(RTCStatsResponseInit, init) {
  EscapableHandleScope scope;
  return Validation<Local<Value>>::Valid(scope.Escape(RTCStatsResponse::Create(init.first, init.second)->handle()));
}

TO_JS(webrtc::RtpSource, source) {
  EscapableHandleScope scope;
  auto object = Nan::New<Object>();
  object->Set(Nan::New("timestamp").ToLocalChecked(), Nan::New<Number>(source.timestamp_ms()));
  object->Set(Nan::New("source").ToLocalChecked(), Nan::New(source.source_id()));
  return Validation<Local<Value>>::Valid(scope.Escape(object));
}

TO_JS(webrtc::RtpHeaderExtensionParameters, params) {
  EscapableHandleScope scope;
  auto object = Nan::New<Object>();
  object->Set(Nan::New("uri").ToLocalChecked(), Nan::New(params.uri).ToLocalChecked());
  object->Set(Nan::New("id").ToLocalChecked(), Nan::New(params.id));
  return Validation<Local<Value>>::Valid(scope.Escape(object));
}

TO_JS(webrtc::RtpCodecParameters, params) {
  EscapableHandleScope scope;
  auto object = Nan::New<Object>();
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
  return Validation<Local<Value>>::Valid(scope.Escape(object));
}

TO_JS(webrtc::RtcpParameters, params) {
  EscapableHandleScope scope;
  auto object = Nan::New<Object>();
  if (!params.cname.empty()) {
    object->Set(Nan::New("cname").ToLocalChecked(), Nan::New(params.cname).ToLocalChecked());
  }
  object->Set(Nan::New("reducedSize").ToLocalChecked(), Nan::New(params.reduced_size));
  return Validation<Local<Value>>::Valid(scope.Escape(Local<Value>::Cast(object)));
}

static Local<Value> CreateRtpParameters(Local<Value> headerExtensions, Local<Value> codecs, Local<Value> rtcp) {
  EscapableHandleScope scope;
  auto object = Nan::New<Object>();
  object->Set(Nan::New("headerExtensions").ToLocalChecked(), headerExtensions);
  object->Set(Nan::New("codecs").ToLocalChecked(), codecs);
  object->Set(Nan::New("encodings").ToLocalChecked(), Nan::New<Array>());
  object->Set(Nan::New("rtcp").ToLocalChecked(), rtcp);
  return scope.Escape(object);
}

TO_JS(webrtc::RtpParameters, params) {
  return curry(CreateRtpParameters)
      % From<Local<Value>>(params.header_extensions)
      * From<Local<Value>>(params.codecs)
      * From<Local<Value>>(params.rtcp);
}

TO_JS(RTCRtpSender*, sender) {
  Nan::EscapableHandleScope scope;
  if (!sender) {
    return Validation<v8::Local<v8::Value>>::Invalid("RTCRtpSender is null");
  }
  return Validation<v8::Local<v8::Value>>::Valid(scope.Escape(sender->ToObject()));
}

FROM_JS(RTCRtpSender*, value) {
  // TODO(mroberts): This is not safe.
  return value->IsObject() && !value->IsNull() && !value->IsArray()
      ? Validation<RTCRtpSender*>::Valid(AsyncObjectWrapWithLoop<RTCRtpSender>::Unwrap(value->ToObject()))
      : Validation<RTCRtpSender*>::Invalid("This is not an RTCRtpSender");
}

TO_JS(RTCRtpReceiver*, receiver) {
  Nan::EscapableHandleScope scope;
  if (!receiver) {
    return Validation<Local<Value>>::Invalid("RTCRtpReceiver is null");
  }
  return Validation<Local<Value>>::Valid(scope.Escape(receiver->ToObject()));
}

TO_JS(RTCRtpTransceiver*, transceiver) {
  Nan::EscapableHandleScope scope;
  if (!transceiver) {
    return Validation<v8::Local<v8::Value>>::Invalid("RTCRtpTransceiver is null");
  }
  return Validation<v8::Local<v8::Value>>::Valid(scope.Escape(transceiver->ToObject()));
}

TO_JS(MediaStream*, stream) {
  Nan::EscapableHandleScope scope;
  if (!stream) {
    return Validation<v8::Local<v8::Value>>::Invalid("MediaStream is null");
  }
  return Validation<v8::Local<v8::Value>>::Valid(scope.Escape(stream->handle()));
}

FROM_JS(MediaStream*, value) {
  // TODO(mroberts): This is not safe.
  return value->IsObject() && !value->IsNull() && !value->IsArray()
      ? Validation<MediaStream*>::Valid(Nan::ObjectWrap::Unwrap<MediaStream>(value->ToObject()))
      : Validation<MediaStream*>::Invalid("This is not a MediaStream");
}

TO_JS(MediaStreamTrack*, track) {
  Nan::EscapableHandleScope scope;
  if (!track) {
    return Validation<v8::Local<v8::Value>>::Invalid("MediaStreamTrack is null");
  }
  return Validation<v8::Local<v8::Value>>::Valid(scope.Escape(track->ToObject()));
}

Validation<std::string> Converter<webrtc::MediaStreamTrackInterface::TrackState, std::string>::Convert(webrtc::MediaStreamTrackInterface::TrackState state) {
  switch (state) {
    case webrtc::MediaStreamTrackInterface::TrackState::kEnded:
      return Validation<std::string>::Valid("ended");
    case webrtc::MediaStreamTrackInterface::TrackState::kLive:
      return Validation<std::string>::Valid("live");
  }
}

FROM_JS(MediaStreamTrack*, value) {
  // TODO(mroberts): This is not safe.
  return value->IsObject() && !value->IsNull() && !value->IsArray()
      ? Validation<MediaStreamTrack*>::Valid(AsyncObjectWrapWithLoop<MediaStreamTrack>::Unwrap(value->ToObject()))
      : Validation<MediaStreamTrack*>::Invalid("This is not a MediaStreamTrack");
}

CONVERTER(webrtc::RtpTransceiverDirection, std::string, value) {
  switch (value) {
    case webrtc::RtpTransceiverDirection::kSendRecv:
      return Validation<std::string>::Valid("sendrecv");
    case webrtc::RtpTransceiverDirection::kSendOnly:
      return Validation<std::string>::Valid("sendonly");
    case webrtc::RtpTransceiverDirection::kRecvOnly:
      return Validation<std::string>::Valid("recvonly");
    case webrtc::RtpTransceiverDirection::kInactive:
      return Validation<std::string>::Valid("inactive");
  }
}

CONVERTER(std::string, webrtc::RtpTransceiverDirection, value) {
  if (value == "sendrecv") {
    return Validation<webrtc::RtpTransceiverDirection>::Valid(webrtc::RtpTransceiverDirection::kSendRecv);
  } else if (value == "sendonly") {
    return Validation<webrtc::RtpTransceiverDirection>::Valid(webrtc::RtpTransceiverDirection::kSendOnly);
  } else if (value == "recvonly") {
    return Validation<webrtc::RtpTransceiverDirection>::Valid(webrtc::RtpTransceiverDirection::kRecvOnly);
  } else if (value == "inactive") {
    return Validation<webrtc::RtpTransceiverDirection>::Valid(webrtc::RtpTransceiverDirection::kInactive);
  }
  return Validation<webrtc::RtpTransceiverDirection>::Invalid(R"(Expected "sendrecv", "sendonly", "recvonly" or "inactive")");
}

TO_JS_ENUM(webrtc::RtpTransceiverDirection)
FROM_JS_ENUM(webrtc::RtpTransceiverDirection)

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

CONVERTER(Local<Value>, webrtc::RtpTransceiverInit, value) {
  return From<Local<Object>>(value).FlatMap<webrtc::RtpTransceiverInit>(
  [](const Local<Object> object) {
    return curry(CreateRtpTransceiverInit)
        % GetOptional<webrtc::RtpTransceiverDirection>(object, "direction", webrtc::RtpTransceiverDirection::kSendRecv)
        * GetOptional<std::vector<node_webrtc::MediaStream*>>(object, "streams", std::vector<node_webrtc::MediaStream*>());
  });
}

TO_JS(const webrtc::RTCStatsMemberInterface*, value) {
  switch (value->type()) {
    case webrtc::RTCStatsMemberInterface::Type::kBool:  // bool
      return From<Local<Value>>(*value->cast_to<webrtc::RTCStatsMember<bool>>());
    case webrtc::RTCStatsMemberInterface::Type::kInt32:  // int32_t
      return From<Local<Value>>(*value->cast_to<webrtc::RTCStatsMember<int32_t>>());
    case webrtc::RTCStatsMemberInterface::Type::kUint32:  // uint32_t
      return From<Local<Value>>(*value->cast_to<webrtc::RTCStatsMember<uint32_t>>());
    case webrtc::RTCStatsMemberInterface::Type::kInt64:   // int64_t
      return From<Local<Value>>(*value->cast_to<webrtc::RTCStatsMember<int64_t>>());
    case webrtc::RTCStatsMemberInterface::Type::kUint64:  // uint64_t
      return From<Local<Value>>(*value->cast_to<webrtc::RTCStatsMember<uint64_t>>());
    case webrtc::RTCStatsMemberInterface::Type::kDouble:  // double
      return From<Local<Value>>(*value->cast_to<webrtc::RTCStatsMember<double>>());
    case webrtc::RTCStatsMemberInterface::Type::kString:  // std::string
      return From<Local<Value>>(*value->cast_to<webrtc::RTCStatsMember<std::string>>());
    case webrtc::RTCStatsMemberInterface::Type::kSequenceBool:  // std::vector<bool>
      return From<Local<Value>>(*value->cast_to<webrtc::RTCStatsMember<std::vector<bool>>>());
    case webrtc::RTCStatsMemberInterface::Type::kSequenceInt32:  // std::vector<int32_t>
      return From<Local<Value>>(*value->cast_to<webrtc::RTCStatsMember<std::vector<int32_t>>>());
    case webrtc::RTCStatsMemberInterface::Type::kSequenceUint32:  // std::vector<uint32_t>
      return From<Local<Value>>(*value->cast_to<webrtc::RTCStatsMember<std::vector<uint32_t>>>());
    case webrtc::RTCStatsMemberInterface::Type::kSequenceInt64:  // std::vector<int64_t>
      return From<Local<Value>>(*value->cast_to<webrtc::RTCStatsMember<std::vector<int64_t>>>());
    case webrtc::RTCStatsMemberInterface::Type::kSequenceUint64:  // std::vector<uint64_t>
      return From<Local<Value>>(*value->cast_to<webrtc::RTCStatsMember<std::vector<uint64_t>>>());
    case webrtc::RTCStatsMemberInterface::Type::kSequenceDouble:  // std::vector<double>
      return From<Local<Value>>(*value->cast_to<webrtc::RTCStatsMember<std::vector<double>>>());
    case webrtc::RTCStatsMemberInterface::Type::kSequenceString:  // std::vector<std::string>
      return From<Local<Value>>(*value->cast_to<webrtc::RTCStatsMember<std::vector<std::string>>>());
  }
}

TO_JS(const webrtc::RTCStats*, value) {
  Nan::EscapableHandleScope scope;
  auto context = Nan::GetCurrentContext();
  auto stats = Nan::New<Object>();
  stats->Set(Nan::New("id").ToLocalChecked(), From<Local<Value>>(value->id()).UnsafeFromValid());
  stats->Set(Nan::New("timestamp").ToLocalChecked(), From<Local<Value>>(value->timestamp_us() / 1000.0).UnsafeFromValid());
  stats->Set(Nan::New("type").ToLocalChecked(), From<Local<Value>>(std::string(value->type())).UnsafeFromValid());
  for (const webrtc::RTCStatsMemberInterface* member : value->Members()) {
    if (member->is_defined()) {
      stats->Set(Nan::New(member->name()).ToLocalChecked(), From<Local<Value>>(member).UnsafeFromValid());
    }
  }
  return Validation<Local<Value>>::Valid(scope.Escape(stats));
}

TO_JS(rtc::scoped_refptr<webrtc::RTCStatsReport>, value) {
  Nan::EscapableHandleScope scope;
  auto context = Nan::GetCurrentContext();
  auto report = v8::Map::New(context->GetIsolate());
  for (const webrtc::RTCStats& stats : *value) {
    report->Set(context, Nan::New(stats.id()).ToLocalChecked(), From<Local<Value>>(&stats).UnsafeFromValid()).IsEmpty();
  }
  return Validation<Local<Value>>::Valid(scope.Escape(Local<Value>::Cast(report)));
}
