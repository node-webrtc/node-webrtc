#pragma once

#include <webrtc/api/peer_connection_interface.h>

// IWYU pragma: no_include "src/enums/macros/impls.h"

#define ICE_TRANSPORTS_TYPE webrtc::PeerConnectionInterface::IceTransportsType
#define ICE_TRANSPORTS_TYPE_NAME "RTCIceTransportPolicy"
#define ICE_TRANSPORTS_TYPE_LIST \
  ENUM_SUPPORTED(ICE_TRANSPORTS_TYPE::kAll, "all") \
  ENUM_SUPPORTED(ICE_TRANSPORTS_TYPE::kRelay, "relay") \
  ENUM_UNSUPPORTED(ICE_TRANSPORTS_TYPE::kNoHost, "no-host", "\"no-host\" is not a valid RTCIceTransportPolicy") \
  ENUM_UNSUPPORTED(ICE_TRANSPORTS_TYPE::kNone, "none", "\"none\" is not a valid RTCIceTransportPolicy")

#define ENUM(X) ICE_TRANSPORTS_TYPE ## X
#include "src/enums/macros/decls.h"
#undef ENUM
