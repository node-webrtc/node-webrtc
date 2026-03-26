#pragma once

#include "src/converters/napi.hh"

namespace webrtc {
struct RtcpParameters;
}

namespace node_webrtc {

DECLARE_TO_AND_FROM_NAPI(webrtc::RtcpParameters)

} // namespace node_webrtc
