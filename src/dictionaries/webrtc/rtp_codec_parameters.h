#pragma once

#include "src/converters/napi.h"

namespace webrtc { struct RtpCodecParameters; }

namespace node_webrtc {

DECLARE_TO_NAPI(webrtc::RtpCodecParameters)

}  // namespace node_webrtc
