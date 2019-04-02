#pragma once

#include "src/converters/napi.h"
#include "src/converters/v8.h"

namespace webrtc { struct RtpParameters; }

namespace node_webrtc {

DECLARE_TO_JS(webrtc::RtpParameters)
DECLARE_TO_NAPI(webrtc::RtpParameters)

}  // namespace node_webrtc
