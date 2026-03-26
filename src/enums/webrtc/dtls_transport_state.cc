#include "src/enums/webrtc/dtls_transport_state.hh"

#define ENUM(X) DTLS_TRANSPORT_STATE##X
#include "src/enums/macros/impls.hh"
#undef ENUM
