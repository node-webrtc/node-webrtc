#include "src/enums/webrtc/sctp_transport_state.h"

#define ENUM(X) SCTP_TRANSPORT_STATE ## X
#include "src/enums/macros/impls.h"
#undef ENUM
