#include "src/enums/webrtc/ice_role.hh"

#define ENUM(X) ICE_ROLE##X
#include "src/enums/macros/impls.hh"
#undef ENUM
