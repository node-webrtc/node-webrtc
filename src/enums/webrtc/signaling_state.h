#pragma once

#include <webrtc/api/peer_connection_interface.h>

// IWYU pragma: no_include "src/enums/macros/impls.h"

#define SIGNALING_STATE webrtc::PeerConnectionInterface::SignalingState
#define SIGNALING_STATE_NAME "RTCSignalingState"
#define SIGNALING_STATE_LIST \
  ENUM_SUPPORTED(SIGNALING_STATE, kStable, "stable") \
  ENUM_SUPPORTED(SIGNALING_STATE, kHaveLocalOffer, "have-local-offer") \
  ENUM_SUPPORTED(SIGNALING_STATE, kHaveRemoteOffer, "have-remote-offer") \
  ENUM_SUPPORTED(SIGNALING_STATE, kHaveLocalPrAnswer, "have-local-pranswer") \
  ENUM_SUPPORTED(SIGNALING_STATE, kHaveRemotePrAnswer, "have-remote-pranswer") \
  ENUM_SUPPORTED(SIGNALING_STATE, kClosed, "closed")

#define ENUM(X) SIGNALING_STATE ## X
#include "src/enums/macros/decls.h"
#undef ENUM
