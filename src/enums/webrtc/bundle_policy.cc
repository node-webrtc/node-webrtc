#include "src/enums/webrtc/bundle_policy.h"

#define ENUM(X) BUNDLE_POLICY ## X
#include "src/enums/macros/impls.h"
#undef ENUM
