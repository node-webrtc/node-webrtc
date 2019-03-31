#pragma once

#include <webrtc/api/peer_connection_interface.h>

// IWYU pragma: no_include "src/enums/macros/impls.h"

#define SDP_SEMANTICS webrtc::SdpSemantics
#define SDP_SEMANTICS_NAME "RTCSdpSemantics"
#define SDP_SEMANTICS_LIST \
  ENUM_SUPPORTED(SDP_SEMANTICS, kPlanB, "plan-b") \
  ENUM_SUPPORTED(SDP_SEMANTICS, kUnifiedPlan, "unified-plan")

#define ENUM(X) SDP_SEMANTICS ## X
#include "src/enums/macros/decls.h"
#undef ENUM
