#pragma once

#include "src/converters/napi.h"
#include "src/converters/v8.h"

namespace webrtc { class RTCStats; }

namespace node_webrtc {

DECLARE_TO_JS(const webrtc::RTCStats*)
DECLARE_TO_NAPI(const webrtc::RTCStats*)

}  // namespace node_webrtc
