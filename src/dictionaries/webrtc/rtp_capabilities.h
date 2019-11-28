#pragma once

#include "src/converters/napi.h"

namespace webrtc { struct RtpCapabilities; }

namespace node_webrtc {

DECLARE_TO_NAPI(webrtc::RtpCapabilities)

}  // namespace node_webrtc
