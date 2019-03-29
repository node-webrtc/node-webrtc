#pragma once

namespace webrtc { struct RtpTransceiverInit; }

#define RTP_TRANSCEIVER_INIT webrtc::RtpTransceiverInit

#define DICT(X) RTP_TRANSCEIVER_INIT ## X
#include "src/dictionaries/macros/decls.h"
#undef DICT
