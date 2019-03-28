#include "src/enums/webrtc/ice_gathering_state.h"

#define ENUM(X) ICE_GATHERING_STATE ## X
#include "src/enums/macros/impls.h"
#undef ENUM
