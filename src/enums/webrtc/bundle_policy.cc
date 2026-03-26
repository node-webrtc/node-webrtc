#include "src/enums/webrtc/bundle_policy.hh"

#define ENUM(X) BUNDLE_POLICY##X
#include "src/enums/macros/impls.hh"
#undef ENUM
