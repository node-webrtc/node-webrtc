#pragma once

#include "src/converters/napi.hh"

namespace webrtc {
struct RtpCodecCapability;
}

namespace node_webrtc {

DECLARE_TO_AND_FROM_NAPI(webrtc::RtpCodecCapability)

} // namespace node_webrtc
