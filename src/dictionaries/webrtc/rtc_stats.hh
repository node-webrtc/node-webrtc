#pragma once

#include "src/converters/napi.hh"

namespace webrtc {
class RTCStats;
}

namespace node_webrtc {

DECLARE_TO_NAPI(const webrtc::RTCStats *)

} // namespace node_webrtc
