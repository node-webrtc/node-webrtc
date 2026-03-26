#include "src/enums/webrtc/track_state.hh"

#define ENUM(X) TRACK_STATE##X
#include "src/enums/macros/impls.hh"
#undef ENUM
