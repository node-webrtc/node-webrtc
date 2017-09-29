/* Copyright (c) 2017 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */

/*
 * This file defines conversion functions between WebRTC and v8 data types. We
 * try to match the W3C-specced Web IDL as closely as possible.
 */

#ifndef SRC_CONVERTERS_WEBRTC_H_
#define SRC_CONVERTERS_WEBRTC_H_

#include <nan.h>

#include "webrtc/api/peerconnectioninterface.h"

#include "src/converters.h"
#include "src/converters/v8.h"
#include "src/functional/validation.h"

namespace node_webrtc {

/*
 * dictionary RTCOAuthCredential {
 *   required DOMString macKey;
 *   required DOMString accessToken;
 * };
 */

// NOTE(mroberts): I've added this to match the Web IDL, but our build of
// WebRTC doesn't currently support this.
struct RTCOAuthCredential {
  RTCOAuthCredential(): macKey(""), accessToken("") {}
  RTCOAuthCredential(const std::string macKey, const std::string accessToken): macKey(macKey), accessToken(accessToken) {}
  const std::string macKey;
  const std::string accessToken;
};

template <>
struct Converter<v8::Local<v8::Value>, RTCOAuthCredential> {
  static Validation<RTCOAuthCredential> Convert(const v8::Local<v8::Value> value);
};

/*
 * enum RTCIceCredentialType {
 *   "password",
 *   "oauth"
 * };
 */

enum RTCIceCredentialType {
  kPassword,
  kOAuth
};

template <>
struct Converter<v8::Local<v8::Value>, RTCIceCredentialType> {
  static Validation<RTCIceCredentialType> Convert(const v8::Local<v8::Value> value);
};

/*
 * dictionary RTCIceServer {
 *   required (DOMString or sequence<DOMString>) urls;
 *            DOMString                          username;
 *            (DOMString or RTCOAuthCredential)  credential;
 *            RTCIceCredentialType               credentialType = "password";
 * };
 */

template <>
struct Converter<v8::Local<v8::Value>, webrtc::PeerConnectionInterface::IceServer> {
  static Validation<webrtc::PeerConnectionInterface::IceServer> Convert(const v8::Local<v8::Value> value);
};

/*
 * enum RTCIceTransportPolicy {
 *   "relay",
 *   "all"
 * };
 */

template <>
struct Converter<v8::Local<v8::Value>, webrtc::PeerConnectionInterface::IceTransportsType> {
  static Validation<webrtc::PeerConnectionInterface::IceTransportsType> Convert(const v8::Local<v8::Value> value);
};

/*
 * enum RTCBundlePolicy {
 *   "balanced",
 *   "max-compat",
 *   "max-bundle"
 * };
 */

template <>
struct Converter<v8::Local<v8::Value>, webrtc::PeerConnectionInterface::BundlePolicy> {
  static Validation<webrtc::PeerConnectionInterface::BundlePolicy> Convert(v8::Local<v8::Value> value);
};

/*
 * enum RTCRtcpMuxPolicy {
 *   // At risk due to lack of implementers' interest.
 *   "negotiate",
 *   "require"
 * };
 */

template <>
struct Converter<v8::Local<v8::Value>, webrtc::PeerConnectionInterface::RtcpMuxPolicy> {
  static Validation<webrtc::PeerConnectionInterface::RtcpMuxPolicy> Convert(v8::Local<v8::Value> value);
};

/*
 * dictionary RTCDtlsFingerprint {
 *   DOMString algorithm;
 *   DOMString value;
 * };
 */

// NOTE(mroberts): I've added this to match the Web IDL, but our build of
// WebRTC doesn't currently support this.
struct RTCDtlsFingerprint {
  RTCDtlsFingerprint(): algorithm(Maybe<std::string>::Nothing()), value(Maybe<std::string>::Nothing()) {}
  RTCDtlsFingerprint(const Maybe<std::string> algorithm, const Maybe<std::string> value): algorithm(algorithm), value(value) {}
  const Maybe<std::string> algorithm;
  const Maybe<std::string> value;
};

template <>
struct Converter<v8::Local<v8::Value>, RTCDtlsFingerprint> {
  static Validation<RTCDtlsFingerprint> Convert(v8::Local<v8::Value> value);
};

/*
 * dictionary RTCConfiguration {
 *   sequence<RTCIceServer>   iceServers;
 *   RTCIceTransportPolicy    iceTransportPolicy = "all";
 *   RTCBundlePolicy          bundlePolicy = "balanced";
 *   RTCRtcpMuxPolicy         rtcpMuxPolicy = "require";
 *   DOMString                peerIdentity;
 *   sequence<RTCCertificate> certificates;
 *   [EnforceRange]
 *   octet                    iceCandidatePoolSize = 0;
 * };
 */

template <>
struct Converter<v8::Local<v8::Value>, webrtc::PeerConnectionInterface::RTCConfiguration> {
  static Validation<webrtc::PeerConnectionInterface::RTCConfiguration> Convert(const v8::Local<v8::Value> value);
};

/*
 * dictionary RTCOfferAnswerOptions {
 *   boolean voiceActivityDetection = true;
 * };
 */

/*
 * dictionary RTCOfferOptions : RTCOfferAnswerOptions {
 *   boolean iceRestart = false;
 * };
 *
 * partial dictionary RTCOfferOptions {
 *   boolean offerToReceiveAudio;
 *   boolean offerToReceiveVideo;
 * };
 */

// NOTE(mroberts): I'm just doing something akin to a newtype here.
struct RTCOfferOptions {
  RTCOfferOptions(): options(webrtc::PeerConnectionInterface::RTCOfferAnswerOptions()) {}
  RTCOfferOptions(const webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options): options(options) {}
  const webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
};

template <>
struct Converter<v8::Local<v8::Value>, RTCOfferOptions> {
  static Validation<RTCOfferOptions> Convert(const v8::Local<v8::Value> value);
};

/*
 * dictionary RTCAnswerOptions : RTCOfferAnswerOptions {
 * };
 */

// NOTE(mroberts): I'm just doing something akin to a newtype here.
struct RTCAnswerOptions {
  RTCAnswerOptions(): options(webrtc::PeerConnectionInterface::RTCOfferAnswerOptions()) {}
  RTCAnswerOptions(const webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options): options(options) {}
  const webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
};

template <>
struct Converter<v8::Local<v8::Value>, RTCAnswerOptions> {
  static Validation<RTCAnswerOptions> Convert(const v8::Local<v8::Value> value);
};

}  // namespace node_webrtc

#endif  // SRC_CONVERTERS_WEBRTC_H_
