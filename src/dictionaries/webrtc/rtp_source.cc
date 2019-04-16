#include "src/dictionaries/webrtc/rtp_source.h"

#include <utility>

#include <node-addon-api/napi.h>
#include <webrtc/api/rtp_receiver_interface.h>

#include "src/dictionaries/macros/napi.h"
#include "src/functional/validation.h"

namespace node_webrtc {

TO_NAPI_IMPL(webrtc::RtpSource, pair) {
  auto env = pair.first;
  Napi::EscapableHandleScope scope(env);
  auto source = pair.second;
  NODE_WEBRTC_CREATE_OBJECT_OR_RETURN(env, object)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "timestamp", source.timestamp_ms())
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "source", source.source_id())
  return Pure(scope.Escape(object));
}

}  // namespace node_webrtc
