#include "src/enums/webrtc/data_state.h"

#define ENUM(X) DATA_STATE ## X
#include "src/enums/macros/impls.h"
#undef ENUM
