#pragma once

#include <webrtc/api/rtp_transceiver_interface.h>

// IWYU pragma: no_include "src/enums/macros/impls.h"

#define RTP_TRANSCEIVER_DIRECTION webrtc::RtpTransceiverDirection
#define RTP_TRANSCEIVER_DIRECTION_NAME "RTCRtpTransceiverDirection"
#define RTP_TRANSCEIVER_DIRECTION_LIST \
  ENUM_SUPPORTED(RTP_TRANSCEIVER_DIRECTION::kSendRecv, "sendrecv") \
  ENUM_SUPPORTED(RTP_TRANSCEIVER_DIRECTION::kSendOnly, "sendonly") \
  ENUM_SUPPORTED(RTP_TRANSCEIVER_DIRECTION::kRecvOnly, "recvonly") \
  ENUM_SUPPORTED(RTP_TRANSCEIVER_DIRECTION::kInactive, "inactive") \
  ENUM_SUPPORTED(RTP_TRANSCEIVER_DIRECTION::kStopped, "stopped")

#define ENUM(X) RTP_TRANSCEIVER_DIRECTION ## X
#include "src/enums/macros/decls.h"
#undef ENUM
