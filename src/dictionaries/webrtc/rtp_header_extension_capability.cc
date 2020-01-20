#include "src/dictionaries/webrtc/rtp_header_extension_capability.h"

#include <utility>

#include <node-addon-api/napi.h>
#include <webrtc/api/rtp_parameters.h>

#include "src/dictionaries/macros/napi.h"
#include "src/functional/validation.h"

namespace node_webrtc {

TO_NAPI_IMPL(webrtc::RtpHeaderExtensionCapability, pair) {
  auto env = pair.first;
  Napi::EscapableHandleScope scope(env);
  auto capabilities = pair.second;
  NODE_WEBRTC_CREATE_OBJECT_OR_RETURN(env, object)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "uri", capabilities.uri)
  return Pure(scope.Escape(object));
}

}  // namespace node_webrtc
