#pragma once

// IWYU pragma: no_include "src/enums/macros/impls.h"

#define RTC_SDP_TYPE RTCSdpType
#define RTC_SDP_TYPE_NAME "RTCSdpType"
#define RTC_SDP_TYPE_LIST \
  SUPPORTED(RTC_SDP_TYPE, kOffer, "offer") \
  SUPPORTED(RTC_SDP_TYPE, kAnswer, "answer") \
  SUPPORTED(RTC_SDP_TYPE, kPrAnswer, "pranswer") \
  SUPPORTED(RTC_SDP_TYPE, kRollback, "rollback")

#define ENUM(X) RTC_SDP_TYPE ## X
#include "src/enums/macros/def.h"
#include "src/enums/macros/decls.h"
#undef ENUM
