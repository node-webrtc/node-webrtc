#include "src/dictionaries/webrtc/rtp_parameters.h"

#include <webrtc/api/rtp_parameters.h>

#include "src/converters/object.h"
#include "src/enums/webrtc/degradation_preference.h"
#include "src/dictionaries/macros/napi.h"
#include "src/dictionaries/webrtc/rtcp_parameters.h"
#include "src/dictionaries/webrtc/rtp_codec_parameters.h"
#include "src/dictionaries/webrtc/rtp_encoding_parameters.h"
#include "src/dictionaries/webrtc/rtp_header_extension_parameters.h"
#include "src/functional/curry.h"
#include "src/functional/operators.h"
#include "src/functional/validation.h"

namespace node_webrtc {

TO_NAPI_IMPL(webrtc::RtpParameters, pair) {
  auto env = pair.first;
  auto parameters = pair.second;
  Napi::EscapableHandleScope scope(env);
  NODE_WEBRTC_CREATE_OBJECT_OR_RETURN(env, object)
  if (!parameters.transaction_id.empty()) {
    NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "transactionId", parameters.transaction_id)
  }
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "headerExtensions", parameters.header_extensions)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "codecs", parameters.codecs)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "rtcp", parameters.rtcp)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "encodings", parameters.encodings)
  // TODO: fix this
  // if (parameters.degradation_preference != webrtc::DegradationPreference::BALANCED) {
  //   NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "degradationPreference", parameters.degradation_preference)
  // }
  return Pure(scope.Escape(object));
}

static webrtc::RtpParameters NapiToRtpParameters(
    std::string transactionId,
    std::vector<webrtc::RtpHeaderExtensionParameters> headerExtensions,
    webrtc::RtcpParameters rtcp,
    std::vector<webrtc::RtpCodecParameters> codecs,
    std::vector<webrtc::RtpEncodingParameters> encodings,
    webrtc::DegradationPreference degradationPreference) {
  webrtc::RtpParameters parameters;
  parameters.transaction_id = transactionId;
  parameters.header_extensions = headerExtensions;
  parameters.rtcp = rtcp;
  parameters.codecs = codecs;
  parameters.encodings = encodings;
  parameters.degradation_preference = degradationPreference;
  return parameters;
}

FROM_NAPI_IMPL(webrtc::RtpParameters, value) {
  return From<Napi::Object>(value).FlatMap<webrtc::RtpParameters>([](auto object) {
    return curry(NapiToRtpParameters)
        % GetRequired<std::string>(object, "transactionId")
        * GetRequired<std::vector<webrtc::RtpHeaderExtensionParameters>>(object, "headerExtensions")
        * GetRequired<webrtc::RtcpParameters>(object, "rtcp")
        * GetRequired<std::vector<webrtc::RtpCodecParameters>>(object, "codecs")
        * GetRequired<std::vector<webrtc::RtpEncodingParameters>>(object, "encodings")
        * GetOptional<webrtc::DegradationPreference>(object, "degradationPreference", webrtc::DegradationPreference::BALANCED);
  });
}

}  // namespace node_webrtc
