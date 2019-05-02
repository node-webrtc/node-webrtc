#include "src/enums/webrtc/ice_role.h"

#define ENUM(X) ICE_ROLE ## X
#include "src/enums/macros/impls.h"
#undef ENUM
