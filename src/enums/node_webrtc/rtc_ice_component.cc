#include "src/enums/node_webrtc/rtc_ice_component.hh"

#define ENUM(X) RTC_ICE_COMPONENT##X
#include "src/enums/macros/impls.hh"
#undef ENUM
