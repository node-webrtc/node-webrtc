#include "src/dictionaries/webrtc/video_frame.hh"

#include <utility>

#include <node-addon-api/napi.h>
#include <webrtc/api/video/video_frame.h>

#include "src/dictionaries/macros/napi.hh"
#include "src/dictionaries/webrtc/video_frame_buffer.hh" // IWYU pragma: keep
#include "src/functional/validation.hh"

namespace node_webrtc {

TO_NAPI_IMPL(webrtc::VideoFrame, pair) {
  auto env = pair.first;
  Napi::EscapableHandleScope scope(env);
  auto value = pair.second;
  NODE_WEBRTC_CREATE_OBJECT_OR_RETURN(env, frame)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, frame, "width", value.width())
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, frame, "height", value.height())
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, frame, "rotation",
                                        static_cast<int>(value.rotation()))
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, frame, "data",
                                        value.video_frame_buffer())
  return Pure(scope.Escape(frame));
}

} // namespace node_webrtc
