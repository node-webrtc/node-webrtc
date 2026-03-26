#include "src/enums/node_webrtc/rtc_priority_type.hh"

#define ENUM(X) RTC_PRIORITY_TYPE##X
#include "src/enums/macros/impls.hh"
#undef ENUM
