#include "src/enums/node_webrtc/binary_type.h"

#define ENUM(X) BINARY_TYPE ## X
#include "src/enums/macros/impls.h"
#undef ENUM
