#pragma once

namespace webrtc { enum class DtlsTransportState; }

// IWYU pragma: no_include <api/dtls_transport_interface.h>
// IWYU pragma: no_include "src/enums/macros/impls.h"

#define DTLS_TRANSPORT_STATE webrtc::DtlsTransportState
#define DTLS_TRANSPORT_STATE_NAME "RTCDtlsTransportState"
#define DTLS_TRANSPORT_STATE_LIST \
  SUPPORTED(DTLS_TRANSPORT_STATE, kNew, "new") \
  SUPPORTED(DTLS_TRANSPORT_STATE, kConnecting, "connecting") \
  SUPPORTED(DTLS_TRANSPORT_STATE, kConnected, "connected") \
  SUPPORTED(DTLS_TRANSPORT_STATE, kClosed, "closed") \
  SUPPORTED(DTLS_TRANSPORT_STATE, kFailed, "failed") \
  UNSUPPORTED(DTLS_TRANSPORT_STATE, kNumValues, "num-values", "\"num-values\" is not a valid RTCDtlsTransportState")

#define ENUM(X) DTLS_TRANSPORT_STATE ## X
#include "src/enums/macros/decls.h"  // IWYU pragma: keep
#undef ENUM
