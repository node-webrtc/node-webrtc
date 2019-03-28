#pragma once

#include <webrtc/api/peer_connection_interface.h>

// IWYU pragma: no_include "src/enums/macros/impls.h"

#define ICE_TRANSPORTS_TYPE webrtc::PeerConnectionInterface::IceTransportsType
#define ICE_TRANSPORTS_TYPE_NAME "RTCIceTransportPolicy"
#define ICE_TRANSPORTS_TYPE_LIST \
  SUPPORTED(ICE_TRANSPORTS_TYPE, kAll, "all") \
  SUPPORTED(ICE_TRANSPORTS_TYPE, kRelay, "relay") \
  UNSUPPORTED(ICE_TRANSPORTS_TYPE, kNoHost, "no-host", "\"no-host\" is not a valid RTCIceTransportPolicy") \
  UNSUPPORTED(ICE_TRANSPORTS_TYPE, kNone, "none", "\"none\" is not a valid RTCIceTransportPolicy")

#define ENUM(X) ICE_TRANSPORTS_TYPE ## X
#include "src/enums/macros/decls.h"  // IWYU pragma: keep
#undef ENUM
