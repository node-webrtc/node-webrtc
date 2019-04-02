#pragma once

#include "src/converters/napi.h"
#include "src/converters/v8.h"

namespace webrtc { class RTCError; }

namespace node_webrtc {

DECLARE_TO_JS(webrtc::RTCError*)
DECLARE_TO_JS(const webrtc::RTCError*)

DECLARE_TO_NAPI(webrtc::RTCError*)
DECLARE_TO_NAPI(const webrtc::RTCError*)

}  // namespace node_webrtc
