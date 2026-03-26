#include "src/enums/webrtc/ice_transport_state.hh"

#define ENUM(X) ICE_TRANSPORT_STATE##X
#include "src/enums/macros/impls.hh"
#undef ENUM
