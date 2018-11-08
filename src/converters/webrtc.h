/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
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

#include <iosfwd>

#include <absl/types/optional.h>
#include <nan.h>  // IWYU pragma: keep
#include <webrtc/api/datachannelinterface.h>
#include <webrtc/api/mediastreaminterface.h>
#include <webrtc/api/mediatypes.h>
#include <webrtc/api/peerconnectioninterface.h>
#include <webrtc/api/rtpparameters.h>
#include <v8.h>  // IWYU pragma: keep

#include "src/converters.h"
#include "src/functional/maybe.h"
#include "src/functional/validation.h"

namespace rtc {

template <class T> class scoped_refptr;

}  // namespace rtc

namespace webrtc {

class IceCandidateInterface;
class RTCError;
class RTCStats;
class RTCStatsMemberInterface;
class RTCStatsReport;
class RtpSource;
enum class RtpTransceiverDirection;
struct RtpTransceiverInit;
enum class SdpSemantics;
class SessionDescriptionInterface;

}  // namespace webrtc;

namespace node_webrtc {

#define CONVERTER(I, O) \
  template <> \
  struct Converter<I, O> { \
    static Validation<O> Convert(I); \
  };

#define TO_JS(T) \
  template <> \
  struct Converter<T, v8::Local<v8::Value>> { \
    static Validation<v8::Local<v8::Value>> Convert(T); \
  };

#define FROM_JS(T) \
  template <> \
  struct Converter<v8::Local<v8::Value>, T> { \
    static Validation<T> Convert(v8::Local<v8::Value>); \
  };

#define TO_AND_FROM_JS(T) \
  template <> \
  struct Converter<T, v8::Local<v8::Value>> { \
    static Validation<v8::Local<v8::Value>> Convert(T); \
  }; \
  \
  template <> \
  struct Converter<v8::Local<v8::Value>, T> { \
    static Validation<T> Convert(v8::Local<v8::Value>); \
  };

class MediaStream;
class MediaStreamTrack;
class RTCRtpReceiver;
class RTCRtpSender;
class RTCRtpTransceiver;
class SomeError;

template <typename T>
struct Converter<absl::optional<T>, v8::Local<v8::Value>> {
  static Validation<v8::Local<v8::Value>> Convert(absl::optional<T> value) {
    if (value) {
      return Converter<T, v8::Local<v8::Value>>::Convert(*value);
    }
    return Validation<v8::Local<v8::Value>>::Valid(Nan::Null());
  }
};

CONVERTER(cricket::MediaType, std::string)
CONVERTER(std::string, cricket::MediaType)
TO_AND_FROM_JS(cricket::MediaType)

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
  RTCOAuthCredential(const std::string& macKey, const std::string& accessToken): macKey(macKey), accessToken(accessToken) {}
  const std::string macKey;
  const std::string accessToken;
};

FROM_JS(RTCOAuthCredential)

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

FROM_JS(RTCIceCredentialType)

/*
 * dictionary RTCIceServer {
 *   required (DOMString or sequence<DOMString>) urls;
 *            DOMString                          username;
 *            (DOMString or RTCOAuthCredential)  credential;
 *            RTCIceCredentialType               credentialType = "password";
 * };
 */

TO_AND_FROM_JS(webrtc::PeerConnectionInterface::IceServer)

/*
 * enum RTCIceTransportPolicy {
 *   "relay",
 *   "all"
 * };
 */

CONVERTER(std::string, webrtc::PeerConnectionInterface::IceTransportsType)
CONVERTER(webrtc::PeerConnectionInterface::IceTransportsType, std::string)
TO_AND_FROM_JS(webrtc::PeerConnectionInterface::IceTransportsType)

/*
 * enum RTCBundlePolicy {
 *   "balanced",
 *   "max-compat",
 *   "max-bundle"
 * };
 */

TO_AND_FROM_JS(webrtc::PeerConnectionInterface::BundlePolicy)

/*
 * enum RTCRtcpMuxPolicy {
 *   // At risk due to lack of implementers' interest.
 *   "negotiate",
 *   "require"
 * };
 */

TO_AND_FROM_JS(webrtc::PeerConnectionInterface::RtcpMuxPolicy)

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
  RTCDtlsFingerprint(const Maybe<std::string>& algorithm, const Maybe<std::string>& value): algorithm(algorithm), value(value) {}
  const Maybe<std::string> algorithm;
  const Maybe<std::string> value;
};

FROM_JS(RTCDtlsFingerprint)

/*
 * dictionary UnsignedShortRange {
 *   unsigned short min;
 *   unsigned short max;
 * }
 */

struct UnsignedShortRange {
  UnsignedShortRange(): min(Maybe<uint16_t>::Nothing()), max(Maybe<uint16_t>::Nothing()) {}
  UnsignedShortRange(const Maybe<uint16_t> min, const Maybe<uint16_t> max): min(min), max(max) {}
  Maybe<uint16_t> min;
  Maybe<uint16_t> max;
};

TO_AND_FROM_JS(UnsignedShortRange)

/*
 * enum SdpSemantics {
 *   "plan-b",
 *   "unified-plan"
 * }
 */

CONVERTER(webrtc::SdpSemantics, std::string)
CONVERTER(std::string, webrtc::SdpSemantics)
TO_AND_FROM_JS(webrtc::SdpSemantics)

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
 *   UnsignedShortRange       portRange;
 *   SdpSemantics             sdpSemantics = "plan-b";
 * };
 */

FROM_JS(webrtc::PeerConnectionInterface::RTCConfiguration)

struct ExtendedRTCConfiguration {
  ExtendedRTCConfiguration(): configuration(webrtc::PeerConnectionInterface::RTCConfiguration()), portRange(UnsignedShortRange()) {}
  ExtendedRTCConfiguration(const webrtc::PeerConnectionInterface::RTCConfiguration configuration, const UnsignedShortRange portRange): configuration(configuration), portRange(portRange) {}
  webrtc::PeerConnectionInterface::RTCConfiguration configuration;
  UnsignedShortRange portRange;
};

TO_AND_FROM_JS(ExtendedRTCConfiguration)

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
  explicit RTCOfferOptions(const webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options): options(options) {}
  const webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
};

FROM_JS(RTCOfferOptions)

/*
 * dictionary RTCAnswerOptions : RTCOfferAnswerOptions {
 * };
 */

// NOTE(mroberts): I'm just doing something akin to a newtype here.
struct RTCAnswerOptions {
  RTCAnswerOptions(): options(webrtc::PeerConnectionInterface::RTCOfferAnswerOptions()) {}
  explicit RTCAnswerOptions(const webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options): options(options) {}
  const webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
};

FROM_JS(RTCAnswerOptions)

/*
 * enum RTCSdpType {
 *   "offer",
 *   "pranswer",
 *   "answer",
 *   "rollback"
 * };
 */

enum RTCSdpType {
  kOffer,
  kPrAnswer,
  kAnswer,
  kRollback
};

CONVERTER(std::string, RTCSdpType)
CONVERTER(RTCSdpType, std::string)

FROM_JS(RTCSdpType)

/*
 * dictionary RTCSessionDescriptionInit {
 *   required RTCSdpType type;
 *            DOMString  sdp = "";
 * };
 */

struct RTCSessionDescriptionInit {
  RTCSessionDescriptionInit(): type(kRollback), sdp("") {}
  RTCSessionDescriptionInit(const RTCSdpType type, const std::string& sdp): type(type), sdp(sdp) {}
  RTCSdpType type;
  std::string sdp;
};


TO_AND_FROM_JS(RTCSessionDescriptionInit)

CONVERTER(webrtc::SessionDescriptionInterface*, RTCSessionDescriptionInit)
CONVERTER(RTCSessionDescriptionInit, webrtc::SessionDescriptionInterface*)

FROM_JS(webrtc::SessionDescriptionInterface*)
TO_JS(const webrtc::SessionDescriptionInterface*)

/*
 * dictionary RTCIceCandidateInit {
 *     DOMString       candidate = "";
 *     DOMString?      sdpMid = null;
 *     unsigned short? sdpMLineIndex = null;
 *     DOMString       usernameFragment;
 * };
 */

FROM_JS(webrtc::IceCandidateInterface*)
TO_JS(const webrtc::IceCandidateInterface*)

/*
 * enum RTCPriorityType {
 *   "very-low",
 *   "low",
 *   "medium",
 *   "high"
 * };
 */

enum RTCPriorityType {
  kVeryLow,
  kLow,
  kMedium,
  kHigh
};

FROM_JS(RTCPriorityType)

/*
 * dictionary RTCDataChannelInit {
 *   boolean         ordered = true;
 *   unsigned short  maxPacketLifeTime;
 *   unsigned short  maxRetransmits;
 *   USVString       protocol = "";
 *   boolean         negotiated = false;
 *   [EnforceRange]
 *   unsigned short  id;
 *   RTCPriorityType priority = "low";
 * };
 */

FROM_JS(webrtc::DataChannelInit)

/*
 * enum RTCSignalingState {
 *   "stable",
 *   "have-local-offer",
 *   "have-remote-offer",
 *   "have-local-pranswer",
 *   "have-remote-pranswer",
 *   "closed"
 * }
 */

TO_JS(webrtc::PeerConnectionInterface::SignalingState)

/*
 * enum RTCIceGatheringState {
 *   "new",
 *   "gathering",
 *   "complete"
 * }
 */

TO_JS(webrtc::PeerConnectionInterface::IceGatheringState)

/*
 * enum RTCIceConnectionState {
 *   "new",
 *   "checking",
 *   "connected",
 *   "completed",
 *   "disconnected",
 *   "failed",
 *   "closed"
 * }
 */

TO_JS(webrtc::PeerConnectionInterface::IceConnectionState)

/*
 * enum RTCDataChannelState {
 *   "connecting",
 *   "open",
 *   "closing",
 *   "closed"
 * }
 */

TO_JS(webrtc::DataChannelInterface::DataState)

/*
 * enum BinaryType {
 *   "blob",
 *   "arraybuffer"
 * }
 */

enum BinaryType {
  kBlob,
  kArrayBuffer,
};

TO_AND_FROM_JS(BinaryType)

TO_JS(webrtc::RTCError*)
TO_JS(const webrtc::RTCError*)

CONVERTER(webrtc::RTCError*, SomeError)
CONVERTER(const webrtc::RTCError*, SomeError)

/*
 * enum RTCPeerConnectionState {
 *  "new",
 *  "connecting",
 *  "connected",
 *  "disconnected",
 *  "failed",
 *  "closed"
 * }
 */

enum RTCPeerConnectionState {
  kNew,
  kConnecting,
  kConnected,
  kDisconnected,
  kFailed,
  kClosed
};

// NOTE(mroberts): This is a hack until we update WebRTC.
CONVERTER(webrtc::PeerConnectionInterface::IceConnectionState, RTCPeerConnectionState)

TO_JS(RTCPeerConnectionState)

typedef std::pair<double, std::vector<std::map<std::string, std::string>>> RTCStatsResponseInit;

TO_JS(RTCStatsResponseInit)

/*
 * dictionary RTCRtpContributingSource {
 *   required DOMHighResTimeStamp timestamp;
 *   required unsigned long       source;
 *            double              audioLevel;
 * };
 */

TO_JS(webrtc::RtpSource)

/*
 * dictionary RTCRtpHeaderExtensionParameters {
 *     required DOMString      uri;
 *     required unsigned short id;
 *              boolean        encrypted = false;
 * };
 */

TO_JS(webrtc::RtpHeaderExtensionParameters)

/*
 * dictionary RTCRtcpParameters {
 *     DOMString cname;
 *     boolean   reducedSize;
 * };
 */

/*
 * dictionary RTCRtpCodecParameters {
 *     octet          payloadType;
 *     DOMString      mimeType;
 *     unsigned long  clockRate;
 *     unsigned short channels;
 *     DOMString      sdpFmtpLine;
 * };
 */

TO_JS(webrtc::RtpCodecParameters)

/*
 * dictionary RTCRtcpParameters {
 *     DOMString cname;
 *     boolean   reducedSize;
 * };
 */

TO_JS(webrtc::RtcpParameters)

/*
 * dictionary RTCRtpParameters {
 *   required sequence<RTCRtpHeaderExtensionParameters> headerExtensions;
 *   required RTCRtcpParameters                         rtcp;
 *   required sequence<RTCRtpCodecParameters>           codecs;
 * };
 */

TO_JS(webrtc::RtpParameters)

/*
 * dictionary RTCRtpCodingParameters {
 *   DOMString rid;
 * };
 */

/*
 * dictionary RTCRtpDecodingParameters: RTCRtpCodingParameters {
 * };
 */

/*
 * dictionary RTCRtpReceiveParameters: RTCRtpParameters {
 *   required sequence<RTCRtpDecodingParameters> encodings;
 * };
 */

TO_JS(node_webrtc::RTCRtpReceiver*)

/*
 * enum MediaStreamTrackState {
 *   "live",
 *   "ended"
 * };
 */

CONVERTER(webrtc::MediaStreamTrackInterface::TrackState, std::string)

TO_AND_FROM_JS(node_webrtc::MediaStream*)

TO_AND_FROM_JS(node_webrtc::MediaStreamTrack*)

TO_AND_FROM_JS(node_webrtc::RTCRtpSender*)

TO_JS(node_webrtc::RTCRtpTransceiver*)

/*
 * enum RTCRtpTransceiverDirection {
 *     "sendrecv",
 *     "sendonly",
 *     "recvonly",
 *     "inactive"
 * };
 */

CONVERTER(webrtc::RtpTransceiverDirection, std::string)
CONVERTER(std::string, webrtc::RtpTransceiverDirection)
TO_AND_FROM_JS(webrtc::RtpTransceiverDirection)

/*
 * dictionary RTCRtpTransceiverInit {
 *     RTCRtpTransceiverDirection         direction = "sendrecv";
 *     sequence<MediaStream>              streams = [];
 *     sequence<RTCRtpEncodingParameters> sendEncodings = [];
 * };
 */

FROM_JS(webrtc::RtpTransceiverInit)

/*
 * interface RTCStatsReport {
 *     readonly maplike<DOMString, object>;
 * };
 */

TO_JS(const webrtc::RTCStatsMemberInterface*)
TO_JS(const webrtc::RTCStats*);
TO_JS(rtc::scoped_refptr<webrtc::RTCStatsReport>);

#undef CONVERTER
#undef TO_JS
#undef FROM_JS
#undef TO_AND_FROM_JS

}  // namespace node_webrtc

#endif  // SRC_CONVERTERS_WEBRTC_H_
