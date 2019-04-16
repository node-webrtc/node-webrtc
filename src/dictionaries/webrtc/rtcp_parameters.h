#pragma once

#include "src/converters/napi.h"

namespace webrtc { struct RtcpParameters; }

namespace node_webrtc {

DECLARE_TO_NAPI(webrtc::RtcpParameters)

}  // namespace node_webrtc
