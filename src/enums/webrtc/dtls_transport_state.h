#pragma once

#include <webrtc/api/dtls_transport_interface.h>

// IWYU pragma: no_include "src/enums/macros/impls.h"

#define DTLS_TRANSPORT_STATE webrtc::DtlsTransportState
#define DTLS_TRANSPORT_STATE_NAME "RTCDtlsTransportState"
#define DTLS_TRANSPORT_STATE_LIST \
  ENUM_SUPPORTED(DTLS_TRANSPORT_STATE::kNew, "new") \
  ENUM_SUPPORTED(DTLS_TRANSPORT_STATE::kConnecting, "connecting") \
  ENUM_SUPPORTED(DTLS_TRANSPORT_STATE::kConnected, "connected") \
  ENUM_SUPPORTED(DTLS_TRANSPORT_STATE::kClosed, "closed") \
  ENUM_SUPPORTED(DTLS_TRANSPORT_STATE::kFailed, "failed") \
  ENUM_UNSUPPORTED(DTLS_TRANSPORT_STATE::kNumValues, "num-values", "\"num-values\" is not a valid RTCDtlsTransportState")

#define ENUM(X) DTLS_TRANSPORT_STATE ## X
#include "src/enums/macros/decls.h"
#undef ENUM
