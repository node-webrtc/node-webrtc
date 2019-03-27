/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <iosfwd>

#include <webrtc/api/data_channel_interface.h>
#include <webrtc/api/media_stream_interface.h>
#include <webrtc/api/media_types.h>
#include <webrtc/api/peer_connection_interface.h>
#include <webrtc/api/video/video_frame_buffer.h>
#include <v8.h>

#include "src/converters.h"
#include "src/functional/validation.h"

#define ENUM_IMPL(ENUM) \
  enum ENUM { \
    ENUM ## _LIST \
  };

#define ENUM_IMPL_VALUE(VALUE) VALUE,

#define DECLARE_TO_AND_FROM_JS_ENUM(ENUM) \
  DECLARE_CONVERTER(ENUM, std::string) \
  DECLARE_CONVERTER(std::string, ENUM) \
  DECLARE_TO_AND_FROM_JS(ENUM)

#define BINARYTYPE BinaryType
#define BINARYTYPE_LIST \
  UNSUPPORTED(BINARYTYPE, kBlob, "blob", "\"blob\" is not supported; see TODO") \
  SUPPORTED(BINARYTYPE, kArrayBuffer, "arraybuffer")

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

namespace webrtc {

enum class RtpTransceiverDirection;
enum class DtlsTransportState;

}  // namespace webrtc

namespace node_webrtc {

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
DECLARE_TO_AND_FROM_JS_ENUM(RTCIceCredentialType)
DECLARE_TO_AND_FROM_JS_ENUM(RTCPeerConnectionState)
DECLARE_TO_AND_FROM_JS_ENUM(RTCPriorityType)
DECLARE_TO_AND_FROM_JS_ENUM(RTCSdpType)

DECLARE_TO_AND_FROM_JS_ENUM(cricket::MediaType)
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
