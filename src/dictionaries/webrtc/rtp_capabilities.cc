#include "src/dictionaries/webrtc/rtp_capabilities.h"

#include <cstdint>
#include <iosfwd>
#include <string>

#include <node-addon-api/napi.h>
#include <webrtc/api/rtp_parameters.h>

#include "src/converters.h"
#include "src/converters/object.h"
#include "src/dictionaries/macros/napi.h"
#include "src/dictionaries/webrtc/rtp_codec_capability.h"
#include "src/dictionaries/webrtc/rtp_header_extension_capability.h"
#include "src/functional/curry.h"
#include "src/functional/maybe.h"
#include "src/functional/operators.h"
#include "src/functional/validation.h"

namespace node_webrtc {

TO_NAPI_IMPL(webrtc::RtpCapabilities, pair) {
  auto env = pair.first;
  Napi::EscapableHandleScope scope(env);
  auto capabilities = pair.second;
  NODE_WEBRTC_CREATE_OBJECT_OR_RETURN(env, object)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "codecs", capabilities.codecs)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "headerExtensions", capabilities.header_extensions)
  return Pure(scope.Escape(object));
}

}  // namespace node_webrtc
