#include "src/enums/webrtc/ice_transports_type.h"

#define ENUM(X) ICE_TRANSPORTS_TYPE ## X
#include "src/enums/macros/impls.h"
#undef ENUM
