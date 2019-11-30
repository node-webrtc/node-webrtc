#pragma once

#include "src/converters/napi.h"

namespace webrtc { struct RtpParameters; }

namespace node_webrtc {

DECLARE_TO_AND_FROM_NAPI(webrtc::RtpParameters)

}  // namespace node_webrtc
