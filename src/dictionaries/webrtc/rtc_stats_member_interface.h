#pragma once

#include "src/converters/napi.h"
#include "src/converters/v8.h"

namespace webrtc { class RTCStatsMemberInterface; }

namespace node_webrtc {

DECLARE_TO_JS(const webrtc::RTCStatsMemberInterface*)
DECLARE_TO_NAPI(const webrtc::RTCStatsMemberInterface*)

}  // namespace node_webrtc
