#include "src/enums/webrtc/dtls_transport_state.h"

#define ENUM(X) DTLS_TRANSPORT_STATE ## X
#include "src/enums/macros/impls.h"
#undef ENUM
