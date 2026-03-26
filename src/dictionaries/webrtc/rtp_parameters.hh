#pragma once

#include "src/converters/napi.hh"

namespace webrtc {
struct RtpParameters;
}

namespace node_webrtc {

DECLARE_TO_AND_FROM_NAPI(webrtc::RtpParameters)

} // namespace node_webrtc
