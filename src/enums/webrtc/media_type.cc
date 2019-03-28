#include "src/enums/webrtc/media_type.h"

#define ENUM(X) MEDIA_TYPE ## X
#include "src/enums/macros/impls.h"
#undef ENUM
