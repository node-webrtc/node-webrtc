#include "src/enums/node_webrtc/rtc_dtx_status.h"

#define ENUM(X) RTC_DTX_STATUS ## X
#include "src/enums/macros/impls.h"
#undef ENUM
