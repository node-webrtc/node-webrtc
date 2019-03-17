/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
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

#ifndef SRC_DECLARE_CONVERTERS_WEBRTC_H_
#define SRC_DECLARE_CONVERTERS_WEBRTC_H_

#include <iosfwd>

#include <absl/types/optional.h>
#include <nan.h>  // IWYU pragma: keep
#include <webrtc/api/mediastreaminterface.h>
#include <webrtc/api/peerconnectioninterface.h>
#include <v8.h>  // IWYU pragma: keep

#include "src/converters.h"
#include "src/converters/enums.h"
#include "src/functional/maybe.h"
#include "src/functional/validation.h"

namespace rtc {

template <class T> class scoped_refptr;

}  // namespace rtc

namespace webrtc {

class I420Buffer;
class I420BufferInterface;
class IceCandidateInterface;
struct DataChannelInit;
class RTCError;
class RTCStats;
class RTCStatsMemberInterface;
class RTCStatsReport;
struct RtcpParameters;
struct RtpCodecParameters;
struct RtpExtension;
struct RtpParameters;
class RtpSource;
enum class RtpTransceiverDirection;
struct RtpTransceiverInit;
enum class SdpSemantics;
class SessionDescriptionInterface;
class VideoFrame;
class VideoFrameBuffer;

}  // namespace webrtc;

namespace node_webrtc {

class I420ImageData;
class MediaStream;  // IWYU pragma: keep
class RgbaImageData;
class SomeError;

#define DECLARE_STRUCT(TYPE) \
  struct TYPE { \
    TYPE ## _LIST \
  };

#define DECLARE_STRUCT_OPTIONAL(TYPE, VAR) node_webrtc::Maybe<TYPE> VAR;

#define DECLARE_STRUCT_REQUIRED(TYPE, VAR) TYPE VAR;

#define EXPAND_DEFAULT_STRUCT(TYPE, VAR) TYPE VAR;

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

#define RTCANSWEROPTIONS RTCAnswerOptions
#define RTCANSWEROPTIONS_LIST \
  DEFAULT(bool, voiceActivityDetection, "voiceActivityDetection", true)

#define RTCOAUTHCREDENTIAL RTCOAuthCredential
#define RTCOAUTHCREDENTIAL_LIST \
  REQUIRED(std::string, macKey, "macKey") \
  REQUIRED(std::string, accessToken, "accessToken")

#define RTCOFFEROPTIONS RTCOfferOptions
#define RTCOFFEROPTIONS_LIST \
  DEFAULT(bool, voiceActivityDetection, "voiceActivityDetection", true) \
  DEFAULT(bool, iceRestart, "iceRestart", false) \
  OPTIONAL(bool, offerToReceiveAudio, "offerToReceiveAudio") \
  OPTIONAL(bool, offerToReceiveVideo, "offerToReceiveVideo")

#define RTCONDATAEVENTDICT RTCOnDataEventDict
#define RTCONDATAEVENTDICT_LIST \
  REQUIRED(uint8_t*, samples, "samples") \
  DEFAULT(uint8_t, bitsPerSample, "bitsPerSample", 16) \
  REQUIRED(uint16_t, sampleRate, "sampleRate") \
  DEFAULT(uint8_t, channelCount, "channelCount", 1) \
  OPTIONAL(uint16_t, numberOfFrames, "numberOfFrames")

#define RTCDTLSFINGERPRINT RTCDtlsFingerprint
#define RTCDTLSFINGERPRINT_LIST \
  OPTIONAL(std::string, algorithm, "algorithm") \
  OPTIONAL(std::string, value, "value")

#define RTCRTPTRANSCEIVERINIT webrtc::RtpTransceiverInit
#define RTCRTPTRANSCEIVERINIT_LIST \
  DEFAULT(webrtc::RtpTransceiverDirection, direciton, "direction", webrtc::RtpTransceiverDirection::kSendRecv) \
  DEFAULT(std::vector<node_webrtc::MediaStream*>, streams, "streams", std::vector<node_webrtc::MediaStream*>())

#define RTCSESSIONDESCRIPTIONINIT RTCSessionDescriptionInit
#define RTCSESSIONDESCRIPTIONINIT_LIST \
  REQUIRED(node_webrtc::RTCSdpType, type, "type") \
  DEFAULT(std::string, sdp, "sdp", "")

#define RTCVIDEOSOURCEINIT RTCVideoSourceInit
#define RTCVIDEOSOURCEINIT_LIST \
  DEFAULT(bool, isScreencast, "isScreencast", false) \
  OPTIONAL(bool, needsDenoising, "needsDenoising")

#define UNSIGNEDSHORTRANGE UnsignedShortRange
#define UNSIGNEDSHORTRANGE_LIST \
  OPTIONAL(uint16_t, min, "min") \
  OPTIONAL(uint16_t, max, "max")

#define REQUIRED(TYPE, VAR, PROP) DECLARE_STRUCT_REQUIRED(TYPE, VAR)
#define OPTIONAL(TYPE, VAR, PROP) DECLARE_STRUCT_OPTIONAL(TYPE, VAR)
#define DEFAULT(TYPE, VAR, PROP, DEFAULT) EXPAND_DEFAULT_STRUCT(TYPE, VAR)
DECLARE_STRUCT(RTCDTLSFINGERPRINT)
DECLARE_STRUCT(RTCOAUTHCREDENTIAL)
DECLARE_STRUCT(RTCONDATAEVENTDICT)
DECLARE_STRUCT(RTCSESSIONDESCRIPTIONINIT)
DECLARE_STRUCT(RTCVIDEOSOURCEINIT)
DECLARE_STRUCT(UNSIGNEDSHORTRANGE)
#undef REQUIRED
#undef OPTIONAL
#undef DEFAULT

struct ExtendedRTCConfiguration {
  ExtendedRTCConfiguration(): configuration(webrtc::PeerConnectionInterface::RTCConfiguration()), portRange(UnsignedShortRange()) {}
  ExtendedRTCConfiguration(const webrtc::PeerConnectionInterface::RTCConfiguration configuration, const UnsignedShortRange portRange): configuration(configuration), portRange(portRange) {}
  webrtc::PeerConnectionInterface::RTCConfiguration configuration;
  UnsignedShortRange portRange;
};

struct RTCOfferOptions {
  RTCOfferOptions(): options(webrtc::PeerConnectionInterface::RTCOfferAnswerOptions()) {}
  explicit RTCOfferOptions(const webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options): options(options) {}
  const webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
};

struct RTCAnswerOptions {
  RTCAnswerOptions(): options(webrtc::PeerConnectionInterface::RTCOfferAnswerOptions()) {}
  explicit RTCAnswerOptions(const webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options): options(options) {}
  const webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
};

// TODO(mroberts): Move this elsewhere.
template <typename T>
struct Converter<absl::optional<T>, v8::Local<v8::Value>> {
  static Validation<v8::Local<v8::Value>> Convert(absl::optional<T> value) {
    if (value) {
      return Converter<T, v8::Local<v8::Value>>::Convert(*value);
    }
    return Pure(Nan::Null().As<v8::Value>());
  }
};

DECLARE_TO_JS(rtc::scoped_refptr<webrtc::RTCStatsReport>)
DECLARE_FROM_JS(webrtc::DataChannelInit)
DECLARE_CONVERTER(webrtc::RTCError*, SomeError)
DECLARE_FROM_JS(webrtc::IceCandidateInterface*)
DECLARE_TO_JS(const webrtc::IceCandidateInterface*)
DECLARE_TO_AND_FROM_JS(webrtc::PeerConnectionInterface::IceServer)
DECLARE_CONVERTER(webrtc::PeerConnectionInterface::IceConnectionState, RTCPeerConnectionState)
DECLARE_FROM_JS(webrtc::PeerConnectionInterface::RTCConfiguration)
DECLARE_TO_JS(webrtc::RtpCodecParameters)
DECLARE_TO_JS(webrtc::RTCError*)
DECLARE_TO_JS(const webrtc::RTCError*)
DECLARE_CONVERTER(const webrtc::RTCError*, SomeError)
DECLARE_TO_JS(webrtc::RtcpParameters)
DECLARE_TO_JS(webrtc::RtpParameters)
DECLARE_TO_JS(webrtc::RtpExtension)
DECLARE_TO_JS(webrtc::RtpSource)
DECLARE_TO_JS(const webrtc::RTCStats*)
DECLARE_TO_JS(const webrtc::RTCStatsMemberInterface*)
DECLARE_FROM_JS(webrtc::RtpTransceiverInit)
DECLARE_FROM_JS(webrtc::SessionDescriptionInterface*)
DECLARE_CONVERTER(webrtc::SessionDescriptionInterface*, RTCSessionDescriptionInit)
DECLARE_TO_JS(const webrtc::SessionDescriptionInterface*)

typedef std::pair<double, std::vector<std::map<std::string, std::string>>> RTCStatsResponseInit;

DECLARE_TO_AND_FROM_JS(ExtendedRTCConfiguration)
DECLARE_FROM_JS(RTCAnswerOptions)
DECLARE_FROM_JS(RTCDtlsFingerprint)
DECLARE_FROM_JS(RTCOAuthCredential)
DECLARE_FROM_JS(RTCOfferOptions)
DECLARE_TO_AND_FROM_JS(RTCOnDataEventDict)
DECLARE_TO_AND_FROM_JS(RTCSessionDescriptionInit)
DECLARE_CONVERTER(RTCSessionDescriptionInit, webrtc::SessionDescriptionInterface*)
DECLARE_TO_JS(RTCStatsResponseInit)
DECLARE_FROM_JS(RTCVideoSourceInit)
DECLARE_TO_AND_FROM_JS(UnsignedShortRange)
DECLARE_TO_JS(webrtc::VideoTrackSourceInterface::Stats)
DECLARE_FROM_JS(rtc::scoped_refptr<webrtc::I420Buffer>)
DECLARE_TO_JS(rtc::scoped_refptr<webrtc::VideoFrameBuffer>)
DECLARE_TO_JS(rtc::scoped_refptr<webrtc::I420BufferInterface>)
DECLARE_TO_JS(webrtc::VideoFrame)
DECLARE_FROM_JS(node_webrtc::I420ImageData)
DECLARE_FROM_JS(node_webrtc::RgbaImageData)

}  // namespace node_webrtc

#endif  // SRC_DECLARE_CONVERTERS_WEBRTC_H_
