#pragma once

#include "src/converters/napi.hh"

namespace webrtc {
class RTCStatsMemberInterface;
}

namespace node_webrtc {

DECLARE_TO_NAPI(const webrtc::RTCStatsMemberInterface *)

} // namespace node_webrtc
