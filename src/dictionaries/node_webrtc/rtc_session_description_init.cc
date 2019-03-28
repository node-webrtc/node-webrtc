#include "src/dictionaries/node_webrtc/rtc_session_description_init.h"

#define DICT(X) RTC_SESSION_DESCRIPTION_INIT ## X
#include "src/dictionaries/macros/impls.h"
#undef DICT
