#include "src/enums/webrtc/rtcp_mux_policy.h"

#define ENUM(X) RTCP_MUX_POLICY ## X
#include "src/enums/macros/impls.h"
#undef ENUM
