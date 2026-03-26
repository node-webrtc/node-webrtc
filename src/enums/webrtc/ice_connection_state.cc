#include "src/enums/webrtc/ice_connection_state.hh"

#define ENUM(X) ICE_CONNECTION_STATE##X
#include "src/enums/macros/impls.hh"
#undef ENUM
