#include "src/enums/webrtc/signaling_state.hh"

#define ENUM(X) SIGNALING_STATE##X
#include "src/enums/macros/impls.hh"
#undef ENUM
