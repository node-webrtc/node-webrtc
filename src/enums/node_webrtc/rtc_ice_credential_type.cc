#include "src/enums/node_webrtc/rtc_ice_credential_type.h"

#define ENUM(X) RTC_ICE_CREDENTIAL_TYPE ## X
#include "src/enums/macros/impls.h"
#undef ENUM
