#include "src/enums/webrtc/data_state.hh"

#define ENUM(X) DATA_STATE##X
#include "src/enums/macros/impls.hh"
#undef ENUM
