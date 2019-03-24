/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/converters/enums.h"

#include "src/converters/v8.h"  // IWYU pragma: keep

using node_webrtc::BinaryType;
using node_webrtc::RTCIceCredentialType;
using node_webrtc::RTCPeerConnectionState;
using node_webrtc::RTCPriorityType;
using node_webrtc::RTCSdpType;

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
