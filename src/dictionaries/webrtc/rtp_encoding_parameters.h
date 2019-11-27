#pragma once

namespace webrtc { struct RtpEncodingParameters; }

#define RTP_ENCODING_PARAMETERS webrtc::RtpEncodingParameters

#define DICT(X) RTP_ENCODING_PARAMETERS ## X
#include "src/dictionaries/macros/decls.h"
#undef DICT
