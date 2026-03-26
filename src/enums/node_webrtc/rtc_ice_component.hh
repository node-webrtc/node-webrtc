#pragma once

// IWYU pragma: no_include "src/enums/macros/impls.hh"

#define RTC_ICE_COMPONENT RTCIceComponent
#define RTC_ICE_COMPONENT_NAME "RTCIceComponent"
#define RTC_ICE_COMPONENT_LIST                                                 \
  ENUM_SUPPORTED(kRtp, "rtp")                                                  \
  ENUM_SUPPORTED(kRtcp, "rtcp")

#define ENUM(X) RTC_ICE_COMPONENT##X
#include "src/enums/macros/def.hh"
// ordering
#include "src/enums/macros/decls.hh"
#undef ENUM
