#pragma once

#include "src/converters/v8.h"

namespace webrtc { struct RtpParameters; }

namespace node_webrtc {

DECLARE_TO_JS(webrtc::RtpParameters)

}  // namespace node_webrtc
