#include "src/enums/node_webrtc/rtc_sdp_type.hh"

#define ENUM(X) RTC_SDP_TYPE##X
#include "src/enums/macros/impls.hh"
#undef ENUM
