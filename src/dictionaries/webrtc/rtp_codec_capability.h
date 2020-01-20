#pragma once

#include "src/converters/napi.h"

namespace webrtc { struct RtpCodecCapability; }

namespace node_webrtc {

DECLARE_TO_AND_FROM_NAPI(webrtc::RtpCodecCapability)

}  // namespace node_webrtc
