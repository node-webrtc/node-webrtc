#include "src/dictionaries/webrtc/rtcp_parameters.h"

#include <iosfwd>
#include <string>
#include <utility>

#include <node-addon-api/napi.h>
#include <webrtc/api/rtp_parameters.h>

#include "src/converters/object.h"
#include "src/dictionaries/macros/napi.h"
#include "src/functional/curry.h"
#include "src/functional/operators.h"
#include "src/functional/validation.h"

namespace node_webrtc {

TO_NAPI_IMPL(webrtc::RtcpParameters, pair) {
  auto env = pair.first;
  Napi::EscapableHandleScope scope(env);
  auto params = pair.second;
  NODE_WEBRTC_CREATE_OBJECT_OR_RETURN(env, object)
  if (!params.cname.empty()) {
    NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "cname", params.cname)
  }
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "reducedSize", params.reduced_size)
  return Pure(scope.Escape(object));
}

static webrtc::RtcpParameters NapiToRtcpParameters(
    std::string cname,
    bool reducedSize) {
  webrtc::RtcpParameters parameters;
  parameters.cname = cname;
  parameters.reduced_size = reducedSize;
  return parameters;
}

FROM_NAPI_IMPL(webrtc::RtcpParameters, value) {
  return From<Napi::Object>(value).FlatMap<webrtc::RtcpParameters>([](auto object) {
    return curry(NapiToRtcpParameters)
        % GetOptional<std::string>(object, "cname", "")
        * GetOptional<bool>(object, "reducedSize", false);
  });
}

}  // namespace node_webrtc
