#pragma once

#include <webrtc/api/peer_connection_interface.h>

// IWYU pragma: no_include "src/enums/macros/impls.h"

#define SIGNALING_STATE webrtc::PeerConnectionInterface::SignalingState
#define SIGNALING_STATE_NAME "RTCSignalingState"
#define SIGNALING_STATE_LIST \
  SUPPORTED(SIGNALING_STATE, kStable, "stable") \
  SUPPORTED(SIGNALING_STATE, kHaveLocalOffer, "have-local-offer") \
  SUPPORTED(SIGNALING_STATE, kHaveRemoteOffer, "have-remote-offer") \
  SUPPORTED(SIGNALING_STATE, kHaveLocalPrAnswer, "have-local-pranswer") \
  SUPPORTED(SIGNALING_STATE, kHaveRemotePrAnswer, "have-remote-pranswer") \
  SUPPORTED(SIGNALING_STATE, kClosed, "closed")

#define ENUM(X) SIGNALING_STATE ## X
#include "src/enums/macros/decls.h"
#undef ENUM
