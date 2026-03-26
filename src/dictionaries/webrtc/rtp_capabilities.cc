#include "src/dictionaries/webrtc/rtp_capabilities.hh"

#include <cstdint>
#include <iosfwd>
#include <string>

#include <node-addon-api/napi.h>
#include <webrtc/api/rtp_parameters.h>

#include "src/converters.hh"
#include "src/converters/object.hh"
#include "src/dictionaries/macros/napi.hh"
#include "src/dictionaries/webrtc/rtp_codec_capability.hh"
#include "src/dictionaries/webrtc/rtp_header_extension_capability.hh"
#include "src/functional/curry.hh"
#include "src/functional/maybe.hh"
#include "src/functional/operators.hh"
#include "src/functional/validation.hh"

namespace node_webrtc {

TO_NAPI_IMPL(webrtc::RtpCapabilities, pair) {
  auto env = pair.first;
  Napi::EscapableHandleScope scope(env);
  auto capabilities = pair.second;
  NODE_WEBRTC_CREATE_OBJECT_OR_RETURN(env, object)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "codecs",
                                        capabilities.codecs)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "headerExtensions",
                                        capabilities.header_extensions)
  return Pure(scope.Escape(object));
}

} // namespace node_webrtc
