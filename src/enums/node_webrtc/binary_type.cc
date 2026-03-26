#include "src/enums/node_webrtc/binary_type.hh"

#define ENUM(X) BINARY_TYPE##X
#include "src/enums/macros/impls.hh"
#undef ENUM
