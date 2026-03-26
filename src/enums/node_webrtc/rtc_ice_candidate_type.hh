#pragma once

// IWYU pragma: no_include "src/enums/macros/impls.hh"

#define RTC_ICE_CANDIDATE_TYPE RTCIceCandidateType
#define RTC_ICE_CANDIDATE_TYPE_NAME "RTCIceCandidateType"
#define RTC_ICE_CANDIDATE_TYPE_LIST                                            \
  ENUM_SUPPORTED(kHost, "host")                                                \
  ENUM_SUPPORTED(kSrflx, "srflx")                                              \
  ENUM_SUPPORTED(kRelay, "relay")                                              \
  ENUM_SUPPORTED(kPrflx, "prflx")

#define ENUM(X) RTC_ICE_CANDIDATE_TYPE##X
#include "src/enums/macros/def.hh"
// ordering
#include "src/enums/macros/decls.hh"
#undef ENUM
