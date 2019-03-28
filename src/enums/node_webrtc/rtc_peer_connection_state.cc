#include "src/enums/node_webrtc/rtc_peer_connection_state.h"

#define ENUM(X) RTC_PEER_CONNECTION_STATE ## X
#include "src/enums/macros/impls.h"
#undef ENUM
