#pragma once

#include "src/converters/v8.h"

namespace webrtc { class VideoFrame; }

namespace node_webrtc {

DECLARE_TO_JS(webrtc::VideoFrame)

}  // namespace node_webrtc
