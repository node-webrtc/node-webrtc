#pragma once

#include "src/converters/napi.hh"

namespace webrtc {
class VideoFrame;
}

namespace node_webrtc {

DECLARE_TO_NAPI(webrtc::VideoFrame)

} // namespace node_webrtc
