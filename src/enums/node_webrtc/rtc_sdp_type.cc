#include "src/enums/node_webrtc/rtc_sdp_type.h"

#define ENUM(X) RTC_SDP_TYPE ## X
#include "src/enums/macros/impls.h"
#undef ENUM
