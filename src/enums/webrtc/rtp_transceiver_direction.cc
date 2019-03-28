#include "src/enums/webrtc/rtp_transceiver_direction.h"

#include <webrtc/api/rtp_transceiver_interface.h>  // IWYU pragma: keep

#define ENUM(X) RTP_TRANSCEIVER_DIRECTION ## X
#include "src/enums/macros/impls.h"
#undef ENUM
