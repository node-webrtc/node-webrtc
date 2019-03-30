#pragma once

#include "src/converters/v8.h"

namespace webrtc { struct RtcpParameters; }

namespace node_webrtc {

DECLARE_TO_JS(webrtc::RtcpParameters)

}  // namespace node_webrtc
