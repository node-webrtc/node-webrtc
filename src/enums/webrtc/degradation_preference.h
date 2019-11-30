#pragma once

#include <webrtc/api/rtp_parameters.h>

// IWYU pragma: no_include "src/enums/macros/impls.h"

#define DEGRADATON_PREFERENCE webrtc::DegradationPreference
#define DEGRADATON_PREFERENCE_NAME "RTCDegradationPreference"
#define DEGRADATON_PREFERENCE_LIST \
  ENUM_UNSUPPORTED(DEGRADATON_PREFERENCE::DISABLED, "disabled", "\"disabled\" is not a valid RTCDegradationPreference") \
  ENUM_SUPPORTED(DEGRADATON_PREFERENCE::MAINTAIN_FRAMERATE, "maintain-framerate") \
  ENUM_SUPPORTED(DEGRADATON_PREFERENCE::MAINTAIN_RESOLUTION, "maintain-resolution") \
  ENUM_SUPPORTED(DEGRADATON_PREFERENCE::BALANCED, "balanced")

#define ENUM(X) DEGRADATON_PREFERENCE ## X
#include "src/enums/macros/decls.h"
#undef ENUM
