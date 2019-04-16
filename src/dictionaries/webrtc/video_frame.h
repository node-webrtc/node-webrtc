#pragma once

#include "src/converters/napi.h"

namespace webrtc { class VideoFrame; }

namespace node_webrtc {

DECLARE_TO_NAPI(webrtc::VideoFrame)

}  // namespace node_webrtc
