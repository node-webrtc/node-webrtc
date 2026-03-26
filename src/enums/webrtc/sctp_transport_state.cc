#include "src/enums/webrtc/sctp_transport_state.hh"

#define ENUM(X) SCTP_TRANSPORT_STATE##X
#include "src/enums/macros/impls.hh"
#undef ENUM
