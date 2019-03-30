#pragma once

#include <webrtc/api/peer_connection_interface.h>

// IWYU pragma: no_include "src/enums/macros/impls.h"

#define ICE_CONNECTION_STATE webrtc::PeerConnectionInterface::IceConnectionState
#define ICE_CONNECTION_STATE_NAME "RTCIceConnectionState"
#define ICE_CONNECTION_STATE_LIST \
  SUPPORTED(ICE_CONNECTION_STATE, kIceConnectionNew, "new") \
  SUPPORTED(ICE_CONNECTION_STATE, kIceConnectionChecking, "checking") \
  SUPPORTED(ICE_CONNECTION_STATE, kIceConnectionClosed, "closed") \
  SUPPORTED(ICE_CONNECTION_STATE, kIceConnectionCompleted, "completed") \
  SUPPORTED(ICE_CONNECTION_STATE, kIceConnectionConnected, "connected") \
  SUPPORTED(ICE_CONNECTION_STATE, kIceConnectionDisconnected, "disconnected") \
  SUPPORTED(ICE_CONNECTION_STATE, kIceConnectionFailed, "failed") \
  UNSUPPORTED(ICE_CONNECTION_STATE, kIceConnectionMax, "max", "\"max\" is not a valid RTCIceConnectionState")

#define ENUM(X) ICE_CONNECTION_STATE ## X
#include "src/enums/macros/decls.h"
#undef ENUM
