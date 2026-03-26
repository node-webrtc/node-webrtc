#pragma once

// IWYU pragma: no_include "src/enums/macros/impls.hh"

#define RTC_DTX_STATUS RTCDtxStatus
#define RTC_DTX_STATUS_NAME "RTCDtxStatus"
#define RTC_DTX_STATUS_LIST                                                    \
  ENUM_SUPPORTED(kDisabled, "disabled")                                        \
  ENUM_SUPPORTED(kEnabled, "enabled")

#define ENUM(X) RTC_DTX_STATUS##X
#include "src/enums/macros/def.hh"
// ordering
#include "src/enums/macros/decls.hh"
#undef ENUM
