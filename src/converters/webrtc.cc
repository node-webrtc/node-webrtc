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
using node_webrtc::Validation;
using v8::Local;
using v8::Object;
using v8::Value;

using BundlePolicy = webrtc::PeerConnectionInterface::BundlePolicy;
using IceServer = webrtc::PeerConnectionInterface::IceServer;
using IceTransportsType = webrtc::PeerConnectionInterface::IceTransportsType;
using RTCConfiguration = webrtc::PeerConnectionInterface::RTCConfiguration;
using RTCOfferAnswerOptions = webrtc::PeerConnectionInterface::RTCOfferAnswerOptions;
using RtcpMuxPolicy = webrtc::PeerConnectionInterface::RtcpMuxPolicy;

static RTCOAuthCredential CreateRTCOAuthCredential(const std::string macKey, const std::string accessToken) {
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
          return Validation<RTCIceCredentialType>::Invalid("Expected \"password\" or \"oauth\"");
        });
}

static IceServer CreateIceServer(
    const Either<std::string, std::vector<std::string>> urlOrUrls,
    const std::string username,
    const std::string credential,
    const RTCIceCredentialType) {
  IceServer iceServer;
  iceServer.uri = urlOrUrls.FromLeft("");
  iceServer.urls = urlOrUrls.FromRight(std::vector<std::string>());
  iceServer.username = username;
  iceServer.password = credential;
  return iceServer;
}

Validation<IceServer> Converter<Local<Value>, IceServer>::Convert(const Local<Value> value) {
  return From<Local<Object>>(value).FlatMap<IceServer>(
      [](const Local<Object> object) {
        return curry(CreateIceServer)
            % GetRequired<Either<std::string, std::vector<std::string>>>(object, "urls")
            * GetOptional<std::string>(object, "username", "")
            * GetOptional<std::string>(object, "credential", "")
            * GetOptional<RTCIceCredentialType>(object, "credentialType", kPassword);
      });
}

Validation<IceTransportsType> Converter<Local<Value>, IceTransportsType>::Convert(const Local<Value> value) {
  return From<std::string>(value).FlatMap<IceTransportsType>(
      [](const std::string string) {
        if (string == "relay") {
          return Validation<IceTransportsType>::Valid(IceTransportsType::kAll);
        } else if (string == "all") {
          return Validation<IceTransportsType>::Valid(IceTransportsType::kRelay);
        }
        return Validation<IceTransportsType>::Invalid("Expected \"all\" or \"relay\"");
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
        return Validation<BundlePolicy>::Invalid("Expected \"balanced\", \"max-compat\" or \"max-bundle\"");
      });
};

Validation<RtcpMuxPolicy> Converter<Local<Value>, RtcpMuxPolicy>::Convert(const Local<Value> value) {
  return From<std::string>(value).FlatMap<RtcpMuxPolicy>(
      [](const std::string string) {
        if (string == "negotiate") {
          return Validation<RtcpMuxPolicy>::Valid(RtcpMuxPolicy::kRtcpMuxPolicyNegotiate);
        } else if (string == "require") {
          return Validation<RtcpMuxPolicy>::Valid(RtcpMuxPolicy::kRtcpMuxPolicyRequire);
        }
        return Validation<RtcpMuxPolicy>::Invalid("Expected \"negotiate\" or \"require\"");
      });
};

static RTCDtlsFingerprint CreateRTCDtlsFingerprint(const Maybe<std::string> algorithm, const Maybe<std::string> value) {
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
    const std::vector<IceServer> iceServers,
    const IceTransportsType iceTransportsPolicy,
    const BundlePolicy bundlePolicy,
    const RtcpMuxPolicy rtcpMuxPolicy,
    const Maybe<std::string>,
    const Maybe<std::vector<Local<Object>>>,
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
};

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
};

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
};
