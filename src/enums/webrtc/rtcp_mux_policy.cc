#include "src/enums/webrtc/rtcp_mux_policy.hh"

#define ENUM(X) RTCP_MUX_POLICY##X
#include "src/enums/macros/impls.hh"
#undef ENUM
