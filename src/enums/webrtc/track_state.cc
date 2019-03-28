#include "src/enums/webrtc/track_state.h"

#define ENUM(X) TRACK_STATE ## X
#include "src/enums/macros/impls.h"
#undef ENUM
