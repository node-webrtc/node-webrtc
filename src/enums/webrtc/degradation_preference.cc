#include "src/enums/webrtc/degradation_preference.hh"

#define ENUM(X) DEGRADATON_PREFERENCE##X
#include "src/enums/macros/impls.hh"
#undef ENUM
