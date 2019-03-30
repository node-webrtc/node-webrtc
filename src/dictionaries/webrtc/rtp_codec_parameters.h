#pragma once

#include "src/converters/v8.h"

namespace webrtc { struct RtpCodecParameters; }

namespace node_webrtc {

DECLARE_TO_JS(webrtc::RtpCodecParameters)

}  // namespace node_webrtc
