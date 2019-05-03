#pragma once

#include <webrtc/api/transport/enums.h>

// IWYU pragma: no_include "src/enums/macros/impls.h"

#define ICE_TRANSPORT_STATE webrtc::IceTransportState
#define ICE_TRANSPORT_STATE_NAME "RTCIceTransportState"
#define ICE_TRANSPORT_STATE_LIST \
  ENUM_SUPPORTED(ICE_TRANSPORT_STATE::kNew, "new") \
  ENUM_SUPPORTED(ICE_TRANSPORT_STATE::kChecking, "checking") \
  ENUM_SUPPORTED(ICE_TRANSPORT_STATE::kConnected, "connected") \
  ENUM_SUPPORTED(ICE_TRANSPORT_STATE::kCompleted, "completed") \
  ENUM_SUPPORTED(ICE_TRANSPORT_STATE::kFailed, "failed") \
  ENUM_SUPPORTED(ICE_TRANSPORT_STATE::kDisconnected, "disconnected") \
  ENUM_SUPPORTED(ICE_TRANSPORT_STATE::kClosed, "closed")

#define ENUM(X) ICE_TRANSPORT_STATE ## X
#include "src/enums/macros/decls.h"
#undef ENUM
