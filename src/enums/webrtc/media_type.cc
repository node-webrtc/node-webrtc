#include "src/enums/webrtc/media_type.h"

#define ENUM(X) CRICKET_MEDIA_TYPE ## X
#include "src/enums/macros/impls.h"
#undef ENUM
