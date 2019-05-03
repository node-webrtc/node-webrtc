#pragma once

#include <webrtc/api/sctp_transport_interface.h>

// IWYU pragma: no_include "src/enums/macros/impls.h"

#define SCTP_TRANSPORT_STATE webrtc::SctpTransportState
#define SCTP_TRANSPORT_STATE_NAME "RTCSctpTransportState"
#define SCTP_TRANSPORT_STATE_LIST \
  ENUM_SUPPORTED(SCTP_TRANSPORT_STATE::kNew, "connecting") \
  ENUM_SUPPORTED(SCTP_TRANSPORT_STATE::kConnecting, "connecting") \
  ENUM_SUPPORTED(SCTP_TRANSPORT_STATE::kConnected, "connected") \
  ENUM_SUPPORTED(SCTP_TRANSPORT_STATE::kClosed, "closed") \
  ENUM_UNSUPPORTED(SCTP_TRANSPORT_STATE::kNumValues, "num-values", "\"num-values\" is not a valid RTCSctpTransportState")

#define ENUM(X) SCTP_TRANSPORT_STATE ## X
#include "src/enums/macros/decls.h"
#undef ENUM
