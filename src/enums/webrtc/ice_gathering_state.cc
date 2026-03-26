#include "src/enums/webrtc/ice_gathering_state.hh"

#define ENUM(X) ICE_GATHERING_STATE##X
#include "src/enums/macros/impls.hh"
#undef ENUM
