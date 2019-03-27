/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/converters/enums.h"

#include <string>

#include <webrtc/api/dtls_transport_interface.h>
#include <webrtc/api/rtp_transceiver_interface.h>

#include "src/converters/v8.h"

#define ENUM_TO_STRING(ENUM) \
  CONVERTER_IMPL(ENUM, std::string, value) { \
    switch (value) { \
        ENUM ## _LIST \
    } \
  } \
  \
  CONVERT_VIA(v8::Local<v8::Value>, std::string, ENUM)

#define ENUM_TO_STRING_SUPPORTED(ENUM, VALUE, STRING) \
  case ENUM::VALUE: \
  return node_webrtc::Pure<std::string>(STRING);

#define ENUM_TO_STRING_UNSUPPORTED(ENUM, VALUE, ERROR) \
  case ENUM::VALUE: \
  return node_webrtc::Validation<std::string>::Invalid(ERROR);

#define STRING_TO_ENUM(ENUM, ERROR) \
  CONVERTER_IMPL(std::string, ENUM, value) { \
    ENUM ## _LIST \
    return node_webrtc::Validation<ENUM>::Invalid(ERROR); \
  } \
  \
  CONVERT_VIA(ENUM, std::string, v8::Local<v8::Value>)

#define STRING_TO_ENUM_SUPPORTED(ENUM, VALUE, STRING) \
  if (value == STRING) { \
    return node_webrtc::Pure(ENUM::VALUE); \
  }

#define STRING_TO_ENUM_UNSUPPORTED(ENUM, STRING, ERROR) \
  if (value == STRING) { \
    return node_webrtc::Validation<ENUM>::Invalid(ERROR); \
  }

#define BUNDLEPOLICY webrtc::PeerConnectionInterface::BundlePolicy
#define BUNDLEPOLICY_LIST \
  SUPPORTED(BUNDLEPOLICY, kBundlePolicyBalanced, "balanced") \
  SUPPORTED(BUNDLEPOLICY, kBundlePolicyMaxCompat, "max-compat") \
  SUPPORTED(BUNDLEPOLICY, kBundlePolicyMaxBundle, "max-bundle")

#define DATASTATE webrtc::DataChannelInterface::DataState
#define DATASTATE_LIST \
  SUPPORTED(DATASTATE, kClosed, "closed") \
  SUPPORTED(DATASTATE, kClosing, "closing") \
  SUPPORTED(DATASTATE, kConnecting, "connecting") \
  SUPPORTED(DATASTATE, kOpen, "open")

#define ICECONNECTIONSTATE webrtc::PeerConnectionInterface::IceConnectionState
#define ICECONNECTIONSTATE_LIST \
  SUPPORTED(ICECONNECTIONSTATE, kIceConnectionNew, "new") \
  SUPPORTED(ICECONNECTIONSTATE, kIceConnectionChecking, "checking") \
  SUPPORTED(ICECONNECTIONSTATE, kIceConnectionClosed, "closed") \
  SUPPORTED(ICECONNECTIONSTATE, kIceConnectionCompleted, "completed") \
  SUPPORTED(ICECONNECTIONSTATE, kIceConnectionConnected, "connected") \
  SUPPORTED(ICECONNECTIONSTATE, kIceConnectionDisconnected, "disconnected") \
  SUPPORTED(ICECONNECTIONSTATE, kIceConnectionFailed, "failed") \
  UNSUPPORTED(ICECONNECTIONSTATE, kIceConnectionMax, "max", "\"max\" is not a valid RTCIceConnectionState")

#define ICEGATHERINGSTATE webrtc::PeerConnectionInterface::IceGatheringState
#define ICEGATHERINGSTATE_LIST \
  SUPPORTED(ICEGATHERINGSTATE, kIceGatheringNew, "new") \
  SUPPORTED(ICEGATHERINGSTATE, kIceGatheringGathering, "gathering") \
  SUPPORTED(ICEGATHERINGSTATE, kIceGatheringComplete, "complete")

#define ICETRANSPORTSTYPE webrtc::PeerConnectionInterface::IceTransportsType
#define ICETRANSPORTSTYPE_LIST \
  SUPPORTED(ICETRANSPORTSTYPE, kAll, "all") \
  SUPPORTED(ICETRANSPORTSTYPE, kRelay, "relay") \
  UNSUPPORTED(ICETRANSPORTSTYPE, kNoHost, "no-host", "\"no-host\" is not a valid RTCIceTransportPolicy") \
  UNSUPPORTED(ICETRANSPORTSTYPE, kNone, "none", "\"none\" is not a valid RTCIceTransportPolicy")

#define MEDIATYPE cricket::MediaType
#define MEDIATYPE_LIST \
  SUPPORTED(MEDIATYPE, MEDIA_TYPE_AUDIO, "audio") \
  SUPPORTED(MEDIATYPE, MEDIA_TYPE_VIDEO, "video") \
  SUPPORTED(MEDIATYPE, MEDIA_TYPE_DATA, "data")

#define RTCDTLSTRANSPORTSTATE webrtc::DtlsTransportState
#define RTCDTLSTRANSPORTSTATE_LIST \
  SUPPORTED(RTCDTLSTRANSPORTSTATE, kNew, "new") \
  SUPPORTED(RTCDTLSTRANSPORTSTATE, kConnecting, "connecting") \
  SUPPORTED(RTCDTLSTRANSPORTSTATE, kConnected, "connected") \
  SUPPORTED(RTCDTLSTRANSPORTSTATE, kClosed, "closed") \
  SUPPORTED(RTCDTLSTRANSPORTSTATE, kFailed, "failed") \
  UNSUPPORTED(RTCDTLSTRANSPORTSTATE, kNumValues, "num-values", "\"num-values\" is not a valid RTCDtlsTransportState")

#define RTCPMUXPOLICY webrtc::PeerConnectionInterface::RtcpMuxPolicy
#define RTCPMUXPOLICY_LIST \
  SUPPORTED(RTCPMUXPOLICY, kRtcpMuxPolicyNegotiate, "negotiate") \
  SUPPORTED(RTCPMUXPOLICY, kRtcpMuxPolicyRequire, "require")

#define RTCRTPTRANSCEIVERDIRECTION webrtc::RtpTransceiverDirection
#define RTCRTPTRANSCEIVERDIRECTION_LIST \
  SUPPORTED(RTCRTPTRANSCEIVERDIRECTION, kSendRecv, "sendrecv") \
  SUPPORTED(RTCRTPTRANSCEIVERDIRECTION, kSendOnly, "sendonly") \
  SUPPORTED(RTCRTPTRANSCEIVERDIRECTION, kRecvOnly, "recvonly") \
  SUPPORTED(RTCRTPTRANSCEIVERDIRECTION, kInactive, "inactive")

#define SDPSEMANTICS webrtc::SdpSemantics
#define SDPSEMANTICS_LIST \
  SUPPORTED(SDPSEMANTICS, kPlanB, "plan-b") \
  SUPPORTED(SDPSEMANTICS, kUnifiedPlan, "unified-plan")

#define SIGNALINGSTATE webrtc::PeerConnectionInterface::SignalingState
#define SIGNALINGSTATE_LIST \
  SUPPORTED(SIGNALINGSTATE, kStable, "stable") \
  SUPPORTED(SIGNALINGSTATE, kHaveLocalOffer, "have-local-offer") \
  SUPPORTED(SIGNALINGSTATE, kHaveRemoteOffer, "have-remote-offer") \
  SUPPORTED(SIGNALINGSTATE, kHaveLocalPrAnswer, "have-local-pranswer") \
  SUPPORTED(SIGNALINGSTATE, kHaveRemotePrAnswer, "have-remote-pranswer") \
  SUPPORTED(SIGNALINGSTATE, kClosed, "closed")

#define TRACKSTATE webrtc::MediaStreamTrackInterface::TrackState
#define TRACKSTATE_LIST \
  SUPPORTED(TRACKSTATE, kEnded, "ended") \
  SUPPORTED(TRACKSTATE, kLive, "live")

#define VIDEO_FRAME_BUFFER_TYPE webrtc::VideoFrameBuffer::Type
#define VIDEO_FRAME_BUFFER_TYPE_LIST \
  UNSUPPORTED(VIDEO_FRAME_BUFFER_TYPE, kNative, "native", "\"native\" is not a valid VideoFrameBufferType") \
  SUPPORTED(VIDEO_FRAME_BUFFER_TYPE, kI420, "I420") \
  SUPPORTED(VIDEO_FRAME_BUFFER_TYPE, kI420A, "I420A") \
  SUPPORTED(VIDEO_FRAME_BUFFER_TYPE, kI444, "I444") \
  SUPPORTED(VIDEO_FRAME_BUFFER_TYPE, kI010, "I010")

namespace node_webrtc {

#define SUPPORTED(ENUM, VALUE, STRING) ENUM_TO_STRING_SUPPORTED(ENUM, VALUE, STRING)
#define UNSUPPORTED(ENUM, VALUE, STRING, ERROR) ENUM_TO_STRING_UNSUPPORTED(ENUM, VALUE, ERROR)
ENUM_TO_STRING(BINARYTYPE)
ENUM_TO_STRING(BUNDLEPOLICY)
ENUM_TO_STRING(DATASTATE)
ENUM_TO_STRING(ICECONNECTIONSTATE)
ENUM_TO_STRING(ICEGATHERINGSTATE)
ENUM_TO_STRING(ICETRANSPORTSTYPE)
ENUM_TO_STRING(MEDIATYPE)
ENUM_TO_STRING(RTCDTLSTRANSPORTSTATE)
ENUM_TO_STRING(RTCICECREDENTIALTYPE)
ENUM_TO_STRING(RTCPEERCONNECTIONSTATE)
ENUM_TO_STRING(RTCPMUXPOLICY)
ENUM_TO_STRING(RTCPRIORITYTYPE)
ENUM_TO_STRING(RTCRTPTRANSCEIVERDIRECTION)
ENUM_TO_STRING(RTCSDPTYPE)
ENUM_TO_STRING(SDPSEMANTICS)
ENUM_TO_STRING(SIGNALINGSTATE)
ENUM_TO_STRING(TRACKSTATE)
ENUM_TO_STRING(VIDEO_FRAME_BUFFER_TYPE)

#undef SUPPORTED
#undef UNSUPPORTED

#define SUPPORTED(ENUM, VALUE, STRING) STRING_TO_ENUM_SUPPORTED(ENUM, VALUE, STRING)
#define UNSUPPORTED(ENUM, VALUE, STRING, ERROR) STRING_TO_ENUM_UNSUPPORTED(ENUM, STRING, ERROR)
STRING_TO_ENUM(BINARYTYPE, "Invalid binaryType")
STRING_TO_ENUM(BUNDLEPOLICY, "Invalid RTCBundlePolicy")
STRING_TO_ENUM(DATASTATE, "Invalid RTCDataChannelState")
STRING_TO_ENUM(ICECONNECTIONSTATE, "Invalid RTCIceConnectionState")
STRING_TO_ENUM(ICEGATHERINGSTATE, "Invalid RTCIceGatheringState")
STRING_TO_ENUM(ICETRANSPORTSTYPE, "Invalid RTCIceTransportPolicy")
STRING_TO_ENUM(MEDIATYPE, "Invalid kind")
STRING_TO_ENUM(RTCDTLSTRANSPORTSTATE, "Invalid RTCDtlsTransportState")
STRING_TO_ENUM(RTCICECREDENTIALTYPE, "Invalid RTCIceCredentialType")
STRING_TO_ENUM(RTCPEERCONNECTIONSTATE, "Invalid RTCPeerConnectionState")
STRING_TO_ENUM(RTCPMUXPOLICY, "Invalid RTCRtcpMuxPolicy")
STRING_TO_ENUM(RTCPRIORITYTYPE, "Invalid RTCPriorityType")
STRING_TO_ENUM(RTCRTPTRANSCEIVERDIRECTION, "Invalid RTCRtpTransceiverDirection")
STRING_TO_ENUM(RTCSDPTYPE, "Invalid RTCSdpType")
STRING_TO_ENUM(SDPSEMANTICS, "Invalid RTCSdpSemantics")
STRING_TO_ENUM(SIGNALINGSTATE, "Invalid RTCSignalingState")
STRING_TO_ENUM(TRACKSTATE, "Invalid MediaStreamTrackState")
STRING_TO_ENUM(VIDEO_FRAME_BUFFER_TYPE, "Invalid VideoFrameBufferType")
#undef SUPPORTED
#undef UNSUPPORTED

}  // namespace node_webrtc
