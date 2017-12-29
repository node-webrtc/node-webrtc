/* Copyright (c) 2017 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/converters/webrtc.h"

#include "src/converters/object.h"

using node_webrtc::Converter;
using node_webrtc::Either;
using node_webrtc::From;
using node_webrtc::GetOptional;
using node_webrtc::GetRequired;
using node_webrtc::Maybe;
using node_webrtc::RTCAnswerOptions;
using node_webrtc::RTCDtlsFingerprint;
using node_webrtc::RTCIceCredentialType;
using node_webrtc::RTCOAuthCredential;
using node_webrtc::RTCOfferOptions;
using node_webrtc::RTCPriorityType ;
using node_webrtc::RTCSdpType;
using node_webrtc::Validation;
using v8::Local;
using v8::Object;
using v8::Value;
using webrtc::DataChannelInit;
using webrtc::IceCandidateInterface;
using webrtc::SessionDescriptionInterface;

using BundlePolicy = webrtc::PeerConnectionInterface::BundlePolicy;
using IceServer = webrtc::PeerConnectionInterface::IceServer;
using IceTransportsType = webrtc::PeerConnectionInterface::IceTransportsType;
using RTCConfiguration = webrtc::PeerConnectionInterface::RTCConfiguration;
using RTCOfferAnswerOptions = webrtc::PeerConnectionInterface::RTCOfferAnswerOptions;
using RtcpMuxPolicy = webrtc::PeerConnectionInterface::RtcpMuxPolicy;
using SdpParseError = webrtc::SdpParseError;

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
    const Either<std::string, std::vector<std::string>>& urlOrUrls,
    const std::string& username,
    const Either<std::string, RTCOAuthCredential>& credential,
    const RTCIceCredentialType credentialType) {
  if (credential.IsRight() || credentialType != RTCIceCredentialType::kPassword) {
    return Validation<IceServer>::Invalid("OAuth is not currently supported");
  }
  IceServer iceServer;
  iceServer.uri = urlOrUrls.FromLeft("");
  iceServer.urls = urlOrUrls.FromRight(std::vector<std::string>());
  iceServer.username = username;
  iceServer.password = credential.UnsafeFromLeft();
  return Validation<IceServer>::Valid(iceServer);
}

Validation<IceServer> Converter<Local<Value>, IceServer>::Convert(const Local<Value> value) {
  return From<Local<Object>>(value).FlatMap<IceServer>(
      [](const Local<Object> object) {
        return Validation<IceServer>::Join(curry(CreateIceServer)
            % GetRequired<Either<std::string, std::vector<std::string>>>(object, "urls")
            * GetOptional<std::string>(object, "username", "")
            * GetOptional<Either<std::string, RTCOAuthCredential>>(object, "credential", Either<std::string, RTCOAuthCredential>::Left(""))
            * GetOptional<RTCIceCredentialType>(object, "credentialType", kPassword));
      });
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

static RTCConfiguration CreateRTCConfiguration(
    const std::vector<IceServer>& iceServers,
    const IceTransportsType iceTransportsPolicy,
    const BundlePolicy bundlePolicy,
    const RtcpMuxPolicy rtcpMuxPolicy,
    const Maybe<std::string>&,
    const Maybe<std::vector<Local<Object>>>&,
    const uint32_t) {
  RTCConfiguration configuration;
  configuration.servers = iceServers;
  configuration.type = iceTransportsPolicy;
  configuration.bundle_policy = bundlePolicy;
  configuration.rtcp_mux_policy = rtcpMuxPolicy;
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
            * GetOptional<uint32_t>(object, "iceCandidatePoolSize", 0);
      });
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

Validation<RTCSdpType> Converter<Local<Value>, RTCSdpType>::Convert(const Local<Value> value) {
  return From<std::string>(value).FlatMap<RTCSdpType>(
      [](const std::string string) {
        if (string == "offer") {
          return Validation<RTCSdpType>::Valid(RTCSdpType::kOffer);
        } else if (string == "pranswer") {
          return Validation<RTCSdpType>::Valid(RTCSdpType::kPrAnswer);
        } else if (string == "answer") {
          return Validation<RTCSdpType>::Valid(RTCSdpType::kAnswer);
        } else if (string == "rollback") {
          return Validation<RTCSdpType>::Valid(RTCSdpType::kRollback);
        }
        return Validation<RTCSdpType>::Invalid(R"(Expected "offer", "pranswer", "answer" or "rollback")");
      });
}

Validation<SessionDescriptionInterface*> CreateSessionDescriptionInterface(
    const RTCSdpType type,
    const std::string sdp) {
  std::string type_;
  switch (type) {
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
  auto description = webrtc::CreateSessionDescription(type_, std::string(sdp), &error);
  if (!description) {
    return Validation<SessionDescriptionInterface*>::Invalid(error.description);
  }
  return Validation<SessionDescriptionInterface*>::Valid(description);
}

Validation<SessionDescriptionInterface*> Converter<Local<Value>, SessionDescriptionInterface*>::Convert(
    const Local<Value> value) {
  return From<Local<Object>>(value).FlatMap<SessionDescriptionInterface*>(
      [](const Local<Object> object) {
        return Validation<SessionDescriptionInterface*>::Join(curry(CreateSessionDescriptionInterface)
            % GetRequired<RTCSdpType>(object, "type")
            * GetOptional<std::string>(object, "sdp", ""));
      });
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
