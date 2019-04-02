#pragma once

#include "src/converters/napi.h"
#include "src/converters/v8.h"

namespace webrtc { class RtpSource; }

namespace node_webrtc {

DECLARE_TO_JS(webrtc::RtpSource)
DECLARE_TO_NAPI(webrtc::RtpSource)

}  // namespace node_webrtc
