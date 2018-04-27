/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/converters/webrtc.h"

#include "src/converters/object.h"
#include "src/rtcstatsresponse.h"

using Nan::EscapableHandleScope;
using node_webrtc::BinaryType;
using node_webrtc::Converter;
using node_webrtc::Either;
using node_webrtc::ExtendedRTCConfiguration;
using node_webrtc::From;
using node_webrtc::GetOptional;
using node_webrtc::GetRequired;
using node_webrtc::Maybe;
using node_webrtc::RTCAnswerOptions;
using node_webrtc::RTCDtlsFingerprint;
using node_webrtc::RTCIceCredentialType;
using node_webrtc::RTCOAuthCredential;
using node_webrtc::RTCOfferOptions;
using node_webrtc::RTCPeerConnectionState;
using node_webrtc::RTCPriorityType ;
using node_webrtc::RTCSdpType;
using node_webrtc::RTCSessionDescriptionInit;
using node_webrtc::RTCStatsResponse;
using node_webrtc::RTCStatsResponseInit;
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

static RTCOAuthCredential CreateRTCOAuthCredential(
    const std::string& macKey,
    const std::string& accessToken) {
  return RTCOAuthCredential(macKey, accessToken);
}

Validation<RTCOAuthCredential> Converter<Local<Value>, RTCOAuthCredential>::Convert(const Local<Value> value) {
  return From<Local<Object>>(value).FlatMap<RTCOAuthCredential>(
  [](const Local<Object> object) {
    return curry(CreateRTCOAuthCredential)
        % GetRequired<std::string>(object, "macKey")
        * GetRequired<std::string>(object, "accessToken");
  });
}

Validation<RTCIceCredentialType> Converter<Local<Value>, RTCIceCredentialType>::Convert(const Local<Value> value) {
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

Validation<IceServer> Converter<Local<Value>, IceServer>::Convert(const Local<Value> value) {
  return From<Local<Object>>(value).FlatMap<IceServer>(
  [](const Local<Object> object) {
    return Validation<IceServer>::Join(curry(CreateIceServer)
            % GetRequired<Either<std::vector<std::string>, std::string>>(object, "urls")
            * GetOptional<std::string>(object, "username", "")
            * GetOptional<Either<std::string, RTCOAuthCredential>>(object, "credential", Either<std::string, RTCOAuthCredential>::Left(""))
            * GetOptional<RTCIceCredentialType>(object, "credentialType", kPassword));
  });
}

Validation<Local<Value>> Converter<IceServer, Local<Value>>::Convert(IceServer iceServer) {
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

Validation<IceTransportsType> Converter<Local<Value>, IceTransportsType>::Convert(const Local<Value> value) {
  return From<std::string>(value).FlatMap<IceTransportsType>(
  [](const std::string string) {
    if (string == "all") {
      return Validation<IceTransportsType>::Valid(IceTransportsType::kAll);
    } else if (string == "relay") {
      return Validation<IceTransportsType>::Valid(IceTransportsType::kRelay);
    }
    return Validation<IceTransportsType>::Invalid(R"(Expected "all" or "relay")");
  });
}

Validation<Local<Value>> Converter<IceTransportsType, Local<Value>>::Convert(const IceTransportsType type) {
  EscapableHandleScope scope;
  switch (type) {
    case IceTransportsType::kAll:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("all").ToLocalChecked()));
    case IceTransportsType::kRelay:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("relay").ToLocalChecked()));
    default:
      return Validation<Local<Value>>::Invalid("Somehow you've set RTCIceTransportPolicy to an unsupported value; please file a bug at https://github.com/js-platform/node-webrtc");
  }
}

Validation<BundlePolicy> Converter<Local<Value>, BundlePolicy>::Convert(const Local<Value> value) {
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

Validation<Local<Value>> Converter<BundlePolicy, Local<Value>>::Convert(const BundlePolicy type) {
  EscapableHandleScope scope;
  switch (type) {
    case BundlePolicy::kBundlePolicyBalanced:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("balanced").ToLocalChecked()));
    case BundlePolicy::kBundlePolicyMaxBundle:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("max-bundle").ToLocalChecked()));
    case BundlePolicy::kBundlePolicyMaxCompat:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("max-compat").ToLocalChecked()));
  }
  return Validation<Local<Value>>::Invalid("Impossible");
}

Validation<RtcpMuxPolicy> Converter<Local<Value>, RtcpMuxPolicy>::Convert(const Local<Value> value) {
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

Validation<Local<Value>> Converter<RtcpMuxPolicy, Local<Value>>::Convert(const RtcpMuxPolicy type) {
  EscapableHandleScope scope;
  switch (type) {
    case RtcpMuxPolicy::kRtcpMuxPolicyNegotiate:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("negotiate").ToLocalChecked()));
    case RtcpMuxPolicy::kRtcpMuxPolicyRequire:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("require").ToLocalChecked()));
  }
  return Validation<Local<Value>>::Invalid("Impossible");
}

static RTCDtlsFingerprint CreateRTCDtlsFingerprint(
    const Maybe<std::string>& algorithm,
    const Maybe<std::string>& value) {
  return RTCDtlsFingerprint(algorithm, value);
}

Validation<RTCDtlsFingerprint> Converter<Local<Value>, RTCDtlsFingerprint>::Convert(const Local<Value> value) {
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

Validation<UnsignedShortRange> Converter<Local<Value>, UnsignedShortRange>::Convert(const Local<Value> value) {
  return From<Local<Object>>(value).FlatMap<UnsignedShortRange>(
  [](const Local<Object> object) {
    return Validation<UnsignedShortRange>::Join(curry(CreateUnsignedShortRange)
            % GetOptional<uint16_t>(object, "min")
            * GetOptional<uint16_t>(object, "max"));
  });
}

Validation<Local<Value>> Converter<UnsignedShortRange, Local<Value>>::Convert(const UnsignedShortRange value) {
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

static RTCConfiguration CreateRTCConfiguration(
    const std::vector<IceServer>& iceServers,
    const IceTransportsType iceTransportsPolicy,
    const BundlePolicy bundlePolicy,
    const RtcpMuxPolicy rtcpMuxPolicy,
    const Maybe<std::string>&,
    const Maybe<std::vector<Local<Object>>>&,
    const uint32_t iceCandidatePoolSize) {
  RTCConfiguration configuration;
  configuration.servers = iceServers;
  configuration.type = iceTransportsPolicy;
  configuration.bundle_policy = bundlePolicy;
  configuration.rtcp_mux_policy = rtcpMuxPolicy;
  configuration.ice_candidate_pool_size = iceCandidatePoolSize;
  return configuration;
}

Validation<RTCConfiguration> Converter<Local<Value>, RTCConfiguration>::Convert(const Local<Value> value) {
  return From<Local<Object>>(value).FlatMap<RTCConfiguration>(
  [](const Local<Object> object) {
    return curry(CreateRTCConfiguration)
        % GetOptional<std::vector<IceServer>>(object, "iceServers", std::vector<IceServer>())
        * GetOptional<IceTransportsType>(object, "iceTransportPolicy", IceTransportsType::kAll)
        * GetOptional<BundlePolicy>(object, "bundlePolicy", BundlePolicy::kBundlePolicyBalanced)
        * GetOptional<RtcpMuxPolicy>(object, "rtcpMuxPolicy", RtcpMuxPolicy::kRtcpMuxPolicyRequire)
        * GetOptional<std::string>(object, "peerIdentity")
        * GetOptional<std::vector<Local<Object>>>(object, "certificates")
        // TODO(mroberts): Implement EnforceRange and change to uint8_t.
        * GetOptional<uint8_t>(object, "iceCandidatePoolSize", 0);
  });
}

static ExtendedRTCConfiguration CreateExtendedRTCConfiguration(
    const RTCConfiguration configuration,
    const UnsignedShortRange portRange) {
  return ExtendedRTCConfiguration(configuration, portRange);
}

Validation<ExtendedRTCConfiguration> Converter<Local<Value>, ExtendedRTCConfiguration>::Convert(const Local<Value> value) {
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
    const Local<Value> portRange) {
  EscapableHandleScope scope;
  auto object = Nan::New<Object>();
  object->Set(Nan::New("iceServers").ToLocalChecked(), iceServers);
  object->Set(Nan::New("iceTransportPolicy").ToLocalChecked(), iceTransportPolicy);
  object->Set(Nan::New("bundlePolicy").ToLocalChecked(), bundlePolicy);
  object->Set(Nan::New("rtcpMuxPolicy").ToLocalChecked(), rtcpMuxPolicy);
  object->Set(Nan::New("iceCandidatePoolSize").ToLocalChecked(), iceCandidatePoolSize);
  object->Set(Nan::New("portRange").ToLocalChecked(), portRange);
  return scope.Escape(object);
}

Validation<Local<Value>> Converter<ExtendedRTCConfiguration, Local<Value>>::Convert(ExtendedRTCConfiguration configuration) {
  return curry(ExtendedRTCConfigurationToJavaScript)
      % From<Local<Value>>(configuration.configuration.servers)
      * From<Local<Value>>(configuration.configuration.type)
      * From<Local<Value>>(configuration.configuration.bundle_policy)
      * From<Local<Value>>(configuration.configuration.rtcp_mux_policy)
      * Validation<Local<Value>>::Valid(Nan::New(configuration.configuration.ice_candidate_pool_size))
      * From<Local<Value>>(configuration.portRange);
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

Validation<RTCOfferOptions> Converter<Local<Value>, RTCOfferOptions>::Convert(const Local<Value> value) {
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

Validation<RTCAnswerOptions> Converter<Local<Value>, RTCAnswerOptions>::Convert(const Local<Value> value) {
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
  return Validation<std::string>::Invalid("Impossible");
}

Validation<RTCSdpType> Converter<Local<Value>, RTCSdpType>::Convert(const Local<Value> value) {
  return From<std::string>(value).FlatMap<RTCSdpType>(Converter<std::string, RTCSdpType>::Convert);
}

RTCSessionDescriptionInit CreateRTCSessionDescriptionInit(
    const RTCSdpType type,
    const std::string sdp) {
  return RTCSessionDescriptionInit(type, sdp);
}

Validation<RTCSessionDescriptionInit> Converter<Local<Value>, RTCSessionDescriptionInit>::Convert(
    const Local<Value> value) {
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
    default: // kRollback
      return Validation<SessionDescriptionInterface*>::Invalid("Rollback is not currently supported");
  }
  SdpParseError error;
  auto description = webrtc::CreateSessionDescription(type_, init.sdp, &error);
  if (!description) {
    return Validation<SessionDescriptionInterface*>::Invalid(error.description);
  }
  return Validation<SessionDescriptionInterface*>::Valid(description);
}

Validation<Local<Value>> Converter<RTCSessionDescriptionInit, Local<Value>>::Convert(
const RTCSessionDescriptionInit init) {
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

Validation<SessionDescriptionInterface*> Converter<Local<Value>, SessionDescriptionInterface*>::Convert(
    const Local<Value> value) {
  return From<RTCSessionDescriptionInit>(value)
      .FlatMap<SessionDescriptionInterface*>(Converter<RTCSessionDescriptionInit, SessionDescriptionInterface*>::Convert);
}

Validation<Local<Value>> Converter<const SessionDescriptionInterface*, Local<Value>>::Convert(const SessionDescriptionInterface* value) {
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

Validation<IceCandidateInterface*> Converter<Local<Value>, IceCandidateInterface*>::Convert(const Local<Value> value) {
  return From<Local<Object>>(value).FlatMap<IceCandidateInterface*>(
  [](const Local<Object> object) {
    return Validation<IceCandidateInterface*>::Join(curry(CreateIceCandidateInterface)
            % GetOptional<std::string>(object, "candidate", "")
            * GetOptional<std::string>(object, "sdpMid", "")
            * GetOptional<int>(object, "sdpMLineIndex", 0)
            * GetOptional<std::string>(object, "usernameFragment"));
  });
}

Validation<Local<Value>> Converter<const IceCandidateInterface*, Local<Value>>::Convert(const IceCandidateInterface* value) {
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

Validation<RTCPriorityType> Converter<Local<Value>, RTCPriorityType>::Convert(const Local<Value> value) {
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

static DataChannelInit CreateDataChannelInit(
    const bool ordered,
    const Maybe<uint32_t> maxPacketLifeTime,
    const Maybe<uint32_t> maxRetransmits,
    const std::string& protocol,
    const bool negotiated,
    const Maybe<uint32_t> id,
    const RTCPriorityType) {
  DataChannelInit init;
  init.ordered = ordered;
  init.maxRetransmitTime = maxPacketLifeTime.Map([](const uint32_t i) { return static_cast<int>(i); }).FromMaybe(-1);
  init.maxRetransmits = maxRetransmits.Map([](const uint32_t i) { return static_cast<int>(i); }).FromMaybe(-1);
  init.protocol = protocol;
  init.negotiated = negotiated;
  init.id = id.Map([](const uint32_t i) { return static_cast<int>(i); }).FromMaybe(-1);
  return init;
}

Validation<DataChannelInit> Converter<Local<Value>, DataChannelInit>::Convert(const Local<Value> value) {
  return From<Local<Object>>(value).FlatMap<DataChannelInit>(
  [](const Local<Object> object) {
    return curry(CreateDataChannelInit)
        % GetOptional<bool>(object, "ordered", true)
        * GetOptional<uint32_t>(object, "maxPacketLifeTime")
        * GetOptional<uint32_t>(object, "maxRetransmits")
        * GetOptional<std::string>(object, "protocol", "")
        * GetOptional<bool>(object, "negotiated", false)
        * GetOptional<uint32_t>(object, "id")
        * GetOptional<RTCPriorityType>(object, "priority", kLow);
  });
}

Validation<Local<Value>> Converter<SignalingState, Local<Value>>::Convert(const SignalingState state) {
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
  return Validation<Local<Value>>::Invalid("Impossible");
}

Validation<Local<Value>> Converter<IceGatheringState, Local<Value>>::Convert(const IceGatheringState state) {
  EscapableHandleScope scope;
  switch (state) {
    case IceGatheringState::kIceGatheringNew:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("new").ToLocalChecked()));
    case IceGatheringState::kIceGatheringGathering:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("gathering").ToLocalChecked()));
    case IceGatheringState::kIceGatheringComplete:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("complete").ToLocalChecked()));
  }
  return Validation<Local<Value>>::Invalid("Impossible");
}

Validation<Local<Value>> Converter<IceConnectionState, Local<Value>>::Convert(const IceConnectionState state) {
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
  return Validation<Local<Value>>::Invalid("Impossible");
}

Validation<Local<Value>> Converter<DataState, Local<Value>>::Convert(const DataState state) {
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
  return Validation<Local<Value>>::Invalid("Impossible");
}

Validation<Local<Value>> Converter<BinaryType, Local<Value>>::Convert(const BinaryType binaryType) {
  EscapableHandleScope scope;
  switch (binaryType) {
    case BinaryType::kArrayBuffer:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("arraybuffer").ToLocalChecked()));
    case BinaryType::kBlob:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::New("blob").ToLocalChecked()));
  }
  return Validation<Local<Value>>::Invalid("Impossible");
}

Validation<BinaryType> Converter<Local<Value>, BinaryType>::Convert(const Local<Value> value) {
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

Validation<Local<Value>> Converter<RTCError*, Local<Value>>::Convert(RTCError* error) {
  EscapableHandleScope scope;
  switch (error->type()) {
    case RTCErrorType::NONE:
      // NOTE: This is odd. This entire RTCError class is odd.
      return Validation<Local<Value>>::Invalid("No error.");
    case RTCErrorType::UNSUPPORTED_PARAMETER:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::Error("InvalidAccessError")));
    case RTCErrorType::INVALID_PARAMETER:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::TypeError("InvalidAccessError")));
    case RTCErrorType::INVALID_RANGE:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::RangeError("RangeError")));
    case RTCErrorType::SYNTAX_ERROR:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::SyntaxError("SyntaxError")));
    case RTCErrorType::INVALID_STATE:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::Error("InvalidStateError")));
    case RTCErrorType::INVALID_MODIFICATION:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::Error("InvalidModificationError")));
    case RTCErrorType::NETWORK_ERROR:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::Error("NetworkError")));
    case RTCErrorType::INTERNAL_ERROR:
    case RTCErrorType::UNSUPPORTED_OPERATION:
    case RTCErrorType::RESOURCE_EXHAUSTED:
      return Validation<Local<Value>>::Valid(scope.Escape(Nan::Error("OperationError")));
  }
  return Validation<Local<Value>>::Invalid("Impossible");
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
  return Validation<RTCPeerConnectionState>::Invalid("Impossible");
}

Validation<Local<Value>> Converter<RTCPeerConnectionState, Local<Value>>::Convert(RTCPeerConnectionState state) {
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
  return Validation<Local<Value>>::Invalid("Impossible");
}

Validation<Local<Value>> Converter<RTCStatsResponseInit, Local<Value>>::Convert(RTCStatsResponseInit init) {
  EscapableHandleScope scope;
  Local<Value> cargv[2];
  cargv[0] = Nan::New<External>(const_cast<void*>(static_cast<const void*>(&init.first)));
  cargv[1] = Nan::New<External>(const_cast<void*>(static_cast<const void*>(&init.second)));
  auto response = static_cast<Local<Value>>(Nan::NewInstance(Nan::New(RTCStatsResponse::constructor), 2, cargv).ToLocalChecked());
  return Validation<Local<Value>>::Valid(scope.Escape(response));
}

Validation<Local<Value>> Converter<webrtc::RtpSource, Local<Value>>::Convert(webrtc::RtpSource source) {
  EscapableHandleScope scope;
  auto object = Nan::New<Object>();
  object->Set(Nan::New("timestamp").ToLocalChecked(), Nan::New<Number>(source.timestamp_ms()));
  object->Set(Nan::New("source").ToLocalChecked(), Nan::New(source.source_id()));
  return Validation<Local<Value>>::Valid(scope.Escape(object));
}

Validation<Local<Value>> Converter<webrtc::RtpHeaderExtensionParameters, Local<Value>>::Convert(webrtc::RtpHeaderExtensionParameters params) {
  EscapableHandleScope scope;
  auto object = Nan::New<Object>();
  object->Set(Nan::New("uri").ToLocalChecked(), Nan::New(params.uri).ToLocalChecked());
  object->Set(Nan::New("id").ToLocalChecked(), Nan::New(params.id));
  return Validation<Local<Value>>::Valid(scope.Escape(object));
}

Validation<Local<Value>> Converter<webrtc::RtpCodecParameters, Local<Value>>::Convert(webrtc::RtpCodecParameters params) {
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

static Local<Value> CreateRtpParameters(Local<Value> headerExtensions, Local<Value> codecs) {
  EscapableHandleScope scope;
  auto object = Nan::New<Object>();
  object->Set(Nan::New("headerExtensions").ToLocalChecked(), headerExtensions);
  object->Set(Nan::New("codecs").ToLocalChecked(), codecs);
  object->Set(Nan::New("encodings").ToLocalChecked(), Nan::New<Array>());
  return scope.Escape(object);
}

Validation<Local<Value>> Converter<webrtc::RtpParameters, v8::Local<v8::Value>>::Convert(webrtc::RtpParameters params) {
  EscapableHandleScope scope;
  return curry(CreateRtpParameters)
      % From<Local<Value>>(params.header_extensions)
      * From<Local<Value>>(params.codecs);
}

Validation<std::string> Converter<webrtc::MediaStreamTrackInterface::TrackState, std::string>::Convert(webrtc::MediaStreamTrackInterface::TrackState state) {
  switch (state) {
    case webrtc::MediaStreamTrackInterface::TrackState::kEnded:
      return Validation<std::string>::Valid("ended");
    case webrtc::MediaStreamTrackInterface::TrackState::kLive:
      return Validation<std::string>::Valid("live");
  }
  return Validation<std::string>::Invalid("Impossible");
}
