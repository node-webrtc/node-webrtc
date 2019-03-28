#include "src/enums/webrtc/dtls_transport_state.h"

#include <webrtc/api/dtls_transport_interface.h>  // IWYU pragma: keep

#define ENUM(X) DTLS_TRANSPORT_STATE ## X
#include "src/enums/macros/impls.h"
#undef ENUM
