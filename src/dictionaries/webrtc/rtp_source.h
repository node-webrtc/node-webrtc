#pragma once

#include "src/converters/napi.h"

namespace webrtc { class RtpSource; }

namespace node_webrtc {

DECLARE_TO_NAPI(webrtc::RtpSource)

}  // namespace node_webrtc
