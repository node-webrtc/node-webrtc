#pragma once

#include <webrtc/api/peer_connection_interface.h>

// IWYU pragma: no_include "src/enums/macros/impls.h"

#define PEER_CONNECTION_STATE webrtc::PeerConnectionInterface::PeerConnectionState
#define PEER_CONNECTION_STATE_NAME "kind"
#define PEER_CONNECTION_STATE_LIST \
  ENUM_SUPPORTED(PEER_CONNECTION_STATE::kNew, "new") \
  ENUM_SUPPORTED(PEER_CONNECTION_STATE::kConnecting, "connecting") \
  ENUM_SUPPORTED(PEER_CONNECTION_STATE::kConnected, "connected") \
  ENUM_SUPPORTED(PEER_CONNECTION_STATE::kDisconnected, "disconnected") \
  ENUM_SUPPORTED(PEER_CONNECTION_STATE::kFailed, "failed") \
  ENUM_SUPPORTED(PEER_CONNECTION_STATE::kClosed, "closed")

#define ENUM(X) PEER_CONNECTION_STATE ## X
#include "src/enums/macros/decls.h"
#undef ENUM
