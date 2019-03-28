#include "src/enums/webrtc/signaling_state.h"

#define ENUM(X) SIGNALING_STATE ## X
#include "src/enums/macros/impls.h"
#undef ENUM
