#pragma once

#include "src/converters/napi.h"

namespace webrtc { class RTCStats; }

namespace node_webrtc {

DECLARE_TO_NAPI(const webrtc::RTCStats*)

}  // namespace node_webrtc
