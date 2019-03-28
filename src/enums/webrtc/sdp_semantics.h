#pragma once

namespace webrtc { enum class SdpSemantics; }

// IWYU pragma: no_include <api/peer_connection_interface.h>
// IWYU pragma: no_include "src/enums/macros/impls.h"

#define SDP_SEMANTICS webrtc::SdpSemantics
#define SDP_SEMANTICS_NAME "RTCSdpSemantics"
#define SDP_SEMANTICS_LIST \
  SUPPORTED(SDP_SEMANTICS, kPlanB, "plan-b") \
  SUPPORTED(SDP_SEMANTICS, kUnifiedPlan, "unified-plan")

#define ENUM(X) SDP_SEMANTICS ## X
#include "src/enums/macros/decls.h"  // IWYU pragma: keep
#undef ENUM
