#include "src/enums/node_webrtc/rtc_ice_candidate_type.h"

#define ENUM(X) RTC_ICE_CANDIDATE_TYPE ## X
#include "src/enums/macros/impls.h"
#undef ENUM
