#pragma once

#include "src/converters/napi.hh"

namespace webrtc {
struct RtpCodecParameters;
}

namespace node_webrtc {

DECLARE_TO_AND_FROM_NAPI(webrtc::RtpCodecParameters)

} // namespace node_webrtc
