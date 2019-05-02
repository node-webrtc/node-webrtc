#include "src/enums/webrtc/peer_connection_state.h"

#define ENUM(X) PEER_CONNECTION_STATE ## X
#include "src/enums/macros/impls.h"
#undef ENUM
