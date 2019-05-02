#include "src/enums/node_webrtc/rtc_ice_component.h"

#define ENUM(X) RTC_ICE_COMPONENT ## X
#include "src/enums/macros/impls.h"
#undef ENUM
