#pragma once

#include <webrtc/api/peer_connection_interface.h>

// IWYU pragma: no_include "src/enums/macros/impls.h"

#define ICE_GATHERING_STATE webrtc::PeerConnectionInterface::IceGatheringState
#define ICE_GATHERING_STATE_NAME "RTCIceGatheringState"
#define ICE_GATHERING_STATE_LIST \
  SUPPORTED(ICE_GATHERING_STATE, kIceGatheringNew, "new") \
  SUPPORTED(ICE_GATHERING_STATE, kIceGatheringGathering, "gathering") \
  SUPPORTED(ICE_GATHERING_STATE, kIceGatheringComplete, "complete")

#define ENUM(X) ICE_GATHERING_STATE ## X
#include "src/enums/macros/decls.h"
#undef ENUM
