#pragma once

#include "src/converters/napi.h"
#include "src/converters/v8.h"

namespace webrtc { struct RtpCodecParameters; }

namespace node_webrtc {

DECLARE_TO_JS(webrtc::RtpCodecParameters)
DECLARE_TO_NAPI(webrtc::RtpCodecParameters)

}  // namespace node_webrtc
