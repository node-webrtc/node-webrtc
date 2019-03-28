#include "src/enums/webrtc/sdp_semantics.h"

#include <webrtc/api/peer_connection_interface.h>  // IWYU pragma: keep

#define ENUM(X) SDP_SEMANTICS ## X
#include "src/enums/macros/impls.h"
#undef ENUM
