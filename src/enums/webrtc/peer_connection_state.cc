#include "src/enums/webrtc/peer_connection_state.hh"

#define ENUM(X) PEER_CONNECTION_STATE##X
#include "src/enums/macros/impls.hh"
#undef ENUM
