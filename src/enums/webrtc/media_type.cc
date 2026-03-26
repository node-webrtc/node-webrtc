#include "src/enums/webrtc/media_type.hh"

#define ENUM(X) CRICKET_MEDIA_TYPE##X
#include "src/enums/macros/impls.hh"
#undef ENUM
