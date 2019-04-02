#pragma once

#include "src/converters/napi.h"
#include "src/converters/v8.h"

namespace webrtc { struct RtcpParameters; }

namespace node_webrtc {

DECLARE_TO_JS(webrtc::RtcpParameters)
DECLARE_TO_NAPI(webrtc::RtcpParameters)

}  // namespace node_webrtc
