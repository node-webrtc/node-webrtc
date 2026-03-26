#include "src/enums/webrtc/ice_transports_type.hh"

#define ENUM(X) ICE_TRANSPORTS_TYPE##X
#include "src/enums/macros/impls.hh"
#undef ENUM
