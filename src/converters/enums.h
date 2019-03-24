/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_CONVERTERS_ENUMS_H_
#define SRC_CONVERTERS_ENUMS_H_

#include <iosfwd>

#include <webrtc/api/datachannelinterface.h>
#include <webrtc/api/dtls_transport_interface.h>
#include <webrtc/api/mediastreaminterface.h>
#include <webrtc/api/mediatypes.h>
#include <webrtc/api/peerconnectioninterface.h>
#include <webrtc/api/rtptransceiverinterface.h>
#include <webrtc/api/video/video_frame_buffer.h>
#include <v8.h>  // IWYU pragma: keep

#include "src/converters.h"
#include "src/functional/validation.h"  // IWYU pragma: keep

#define ENUM_IMPL(ENUM) \
  enum ENUM { \
    ENUM ## _LIST \
  };

#define ENUM_IMPL_VALUE(VALUE) VALUE,

#define DECLARE_TO_AND_FROM_JS_ENUM(ENUM) \
  DECLARE_CONVERTER(ENUM, std::string) \
  DECLARE_CONVERTER(std::string, ENUM) \
  DECLARE_TO_AND_FROM_JS(ENUM)

namespace node_webrtc {

#define BINARYTYPE BinaryType
#define BINARYTYPE_LIST \
  UNSUPPORTED(BINARYTYPE, kBlob, "blob", "\"blob\" is not supported; see TODO") \
  SUPPORTED(BINARYTYPE, kArrayBuffer, "arraybuffer")

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

#define RTCICECREDENTIALTYPE RTCIceCredentialType
#define RTCICECREDENTIALTYPE_LIST \
  SUPPORTED(RTCICECREDENTIALTYPE, kPassword, "password") \
  SUPPORTED(RTCICECREDENTIALTYPE, kOAuth, "oauth")

#define RTCPEERCONNECTIONSTATE RTCPeerConnectionState
#define RTCPEERCONNECTIONSTATE_LIST \
  SUPPORTED(RTCPEERCONNECTIONSTATE, kClosed, "closed") \
  SUPPORTED(RTCPEERCONNECTIONSTATE, kConnected, "connected") \
  SUPPORTED(RTCPEERCONNECTIONSTATE, kConnecting, "connecting") \
  SUPPORTED(RTCPEERCONNECTIONSTATE, kDisconnected, "disconnected") \
  SUPPORTED(RTCPEERCONNECTIONSTATE, kFailed, "failed") \
  SUPPORTED(RTCPEERCONNECTIONSTATE, kNew, "new")

#define RTCPMUXPOLICY webrtc::PeerConnectionInterface::RtcpMuxPolicy
#define RTCPMUXPOLICY_LIST \
  SUPPORTED(RTCPMUXPOLICY, kRtcpMuxPolicyNegotiate, "negotiate") \
  SUPPORTED(RTCPMUXPOLICY, kRtcpMuxPolicyRequire, "require")

#define RTCPRIORITYTYPE RTCPriorityType
#define RTCPRIORITYTYPE_LIST \
  SUPPORTED(RTCPRIORITYTYPE, kVeryLow, "very-low") \
  SUPPORTED(RTCPRIORITYTYPE, kLow, "low") \
  SUPPORTED(RTCPRIORITYTYPE, kMedium, "medium") \
  SUPPORTED(RTCPRIORITYTYPE, kHigh, "high")

#define RTCSDPTYPE RTCSdpType
#define RTCSDPTYPE_LIST \
  SUPPORTED(RTCSDPTYPE, kOffer, "offer") \
  SUPPORTED(RTCSDPTYPE, kAnswer, "answer") \
  SUPPORTED(RTCSDPTYPE, kPrAnswer, "pranswer") \
  SUPPORTED(RTCSDPTYPE, kRollback, "rollback")

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

#define SUPPORTED(ENUM, VALUE, STRING) ENUM_IMPL_VALUE(VALUE)
#define UNSUPPORTED(ENUM, VALUE, STRING, ERROR) ENUM_IMPL_VALUE(VALUE)
ENUM_IMPL(BINARYTYPE)
ENUM_IMPL(RTCICECREDENTIALTYPE)
ENUM_IMPL(RTCPEERCONNECTIONSTATE)
ENUM_IMPL(RTCPRIORITYTYPE)
ENUM_IMPL(RTCSDPTYPE)
#undef SUPPORTED
#undef UNSUPPORTED

DECLARE_TO_AND_FROM_JS_ENUM(BinaryType)
DECLARE_TO_AND_FROM_JS_ENUM(cricket::MediaType)
DECLARE_TO_AND_FROM_JS_ENUM(RTCIceCredentialType)
DECLARE_TO_AND_FROM_JS_ENUM(RTCPeerConnectionState)
DECLARE_TO_AND_FROM_JS_ENUM(RTCPriorityType)
DECLARE_TO_AND_FROM_JS_ENUM(RTCSdpType)
DECLARE_TO_AND_FROM_JS_ENUM(webrtc::MediaStreamTrackInterface::TrackState)
DECLARE_TO_AND_FROM_JS_ENUM(webrtc::PeerConnectionInterface::BundlePolicy)
DECLARE_TO_AND_FROM_JS_ENUM(webrtc::DataChannelInterface::DataState)
DECLARE_TO_AND_FROM_JS_ENUM(webrtc::DtlsTransportState)
DECLARE_TO_AND_FROM_JS_ENUM(webrtc::PeerConnectionInterface::IceConnectionState)
DECLARE_TO_AND_FROM_JS_ENUM(webrtc::PeerConnectionInterface::IceGatheringState)
DECLARE_TO_AND_FROM_JS_ENUM(webrtc::PeerConnectionInterface::IceTransportsType)
DECLARE_TO_AND_FROM_JS_ENUM(webrtc::PeerConnectionInterface::RtcpMuxPolicy)
DECLARE_TO_AND_FROM_JS_ENUM(webrtc::PeerConnectionInterface::SignalingState)
DECLARE_TO_AND_FROM_JS_ENUM(webrtc::RtpTransceiverDirection)
DECLARE_TO_AND_FROM_JS_ENUM(webrtc::SdpSemantics)
DECLARE_TO_AND_FROM_JS_ENUM(webrtc::MediaSourceInterface::SourceState)
DECLARE_TO_AND_FROM_JS_ENUM(webrtc::VideoFrameBuffer::Type)

}  // namespace node_webrtc

#endif  // SRC_CONVERTERS_ENUMS_H_
