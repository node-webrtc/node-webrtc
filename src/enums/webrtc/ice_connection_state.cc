#include "src/enums/webrtc/ice_connection_state.h"

#define ENUM(X) ICE_CONNECTION_STATE ## X
#include "src/enums/macros/impls.h"
#undef ENUM
