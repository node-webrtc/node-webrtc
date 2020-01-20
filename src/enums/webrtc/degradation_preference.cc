#include "src/enums/webrtc/degradation_preference.h"

#define ENUM(X) DEGRADATON_PREFERENCE ## X
#include "src/enums/macros/impls.h"
#undef ENUM
