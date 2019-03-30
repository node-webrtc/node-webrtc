#pragma once

// IWYU pragma: no_include "src/enums/macros/impls.h"

#define RTC_PRIORITY_TYPE RTCPriorityType
#define RTC_PRIORITY_TYPE_NAME "RTCPriorityType"
#define RTC_PRIORITY_TYPE_LIST \
  SUPPORTED(RTC_PRIORITY_TYPE, kVeryLow, "very-low") \
  SUPPORTED(RTC_PRIORITY_TYPE, kLow, "low") \
  SUPPORTED(RTC_PRIORITY_TYPE, kMedium, "medium") \
  SUPPORTED(RTC_PRIORITY_TYPE, kHigh, "high")

#define ENUM(X) RTC_PRIORITY_TYPE ## X
#include "src/enums/macros/def.h"
#include "src/enums/macros/decls.h"
#undef ENUM
