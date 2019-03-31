#pragma once

// IWYU pragma: no_include "src/enums/macros/impls.h"

#define RTC_SDP_TYPE RTCSdpType
#define RTC_SDP_TYPE_NAME "RTCSdpType"
#define RTC_SDP_TYPE_LIST \
  ENUM_SUPPORTED(RTC_SDP_TYPE, kOffer, "offer") \
  ENUM_SUPPORTED(RTC_SDP_TYPE, kAnswer, "answer") \
  ENUM_SUPPORTED(RTC_SDP_TYPE, kPrAnswer, "pranswer") \
  ENUM_SUPPORTED(RTC_SDP_TYPE, kRollback, "rollback")

#define ENUM(X) RTC_SDP_TYPE ## X
#include "src/enums/macros/def.h"
#include "src/enums/macros/decls.h"
#undef ENUM
