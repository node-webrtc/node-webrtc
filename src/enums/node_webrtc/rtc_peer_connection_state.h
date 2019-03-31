#pragma once

#include <webrtc/api/peer_connection_interface.h>

#include "src/converters.h"

// IWYU pragma: no_include "src/enums/macros/impls.h"

#define RTC_PEER_CONNECTION_STATE RTCPeerConnectionState
#define RTC_PEER_CONNECTION_STATE_NAME "RTCPeerConnectionState"
#define RTC_PEER_CONNECTION_STATE_LIST \
  ENUM_SUPPORTED(kClosed, "closed") \
  ENUM_SUPPORTED(kConnected, "connected") \
  ENUM_SUPPORTED(kConnecting, "connecting") \
  ENUM_SUPPORTED(kDisconnected, "disconnected") \
  ENUM_SUPPORTED(kFailed, "failed") \
  ENUM_SUPPORTED(kNew, "new")

#define ENUM(X) RTC_PEER_CONNECTION_STATE ## X
#include "src/enums/macros/def.h"
#include "src/enums/macros/decls.h"
#undef ENUM

namespace node_webrtc {

DECLARE_CONVERTER(webrtc::PeerConnectionInterface::IceConnectionState, RTCPeerConnectionState)

}  // namespace node_webrtc
