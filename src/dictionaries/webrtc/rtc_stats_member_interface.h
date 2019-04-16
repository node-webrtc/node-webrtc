#pragma once

#include "src/converters/napi.h"

namespace webrtc { class RTCStatsMemberInterface; }

namespace node_webrtc {

DECLARE_TO_NAPI(const webrtc::RTCStatsMemberInterface*)

}  // namespace node_webrtc
