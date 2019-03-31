#pragma once

// IWYU pragma: no_include "src/enums/macros/impls.h"

#define RTC_PRIORITY_TYPE RTCPriorityType
#define RTC_PRIORITY_TYPE_NAME "RTCPriorityType"
#define RTC_PRIORITY_TYPE_LIST \
  ENUM_SUPPORTED(RTC_PRIORITY_TYPE, kVeryLow, "very-low") \
  ENUM_SUPPORTED(RTC_PRIORITY_TYPE, kLow, "low") \
  ENUM_SUPPORTED(RTC_PRIORITY_TYPE, kMedium, "medium") \
  ENUM_SUPPORTED(RTC_PRIORITY_TYPE, kHigh, "high")

#define ENUM(X) RTC_PRIORITY_TYPE ## X
#include "src/enums/macros/def.h"
#include "src/enums/macros/decls.h"
#undef ENUM
