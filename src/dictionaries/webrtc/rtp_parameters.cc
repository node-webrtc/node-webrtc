#include "src/dictionaries/webrtc/rtp_parameters.hh"

#include <webrtc/api/rtp_parameters.h>

#include "src/converters/absl.hh"
#include "src/converters/object.hh"
#include "src/dictionaries/macros/napi.hh"
#include "src/dictionaries/webrtc/rtcp_parameters.hh"
#include "src/dictionaries/webrtc/rtp_codec_parameters.hh"
#include "src/dictionaries/webrtc/rtp_encoding_parameters.hh"
#include "src/dictionaries/webrtc/rtp_header_extension_parameters.hh"
#include "src/enums/webrtc/degradation_preference.hh"
#include "src/functional/curry.hh"
#include "src/functional/maybe.hh"
#include "src/functional/operators.hh"
#include "src/functional/validation.hh"

namespace node_webrtc {

TO_NAPI_IMPL(webrtc::RtpParameters, pair) {
  auto env = pair.first;
  auto parameters = pair.second;
  Napi::EscapableHandleScope scope(env);
  NODE_WEBRTC_CREATE_OBJECT_OR_RETURN(env, object)
  if (!parameters.transaction_id.empty()) {
    NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "transactionId",
                                          parameters.transaction_id)
  }
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "headerExtensions",
                                        parameters.header_extensions)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "codecs",
                                        parameters.codecs)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "rtcp", parameters.rtcp)

  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "encodings",
                                        parameters.encodings)
  if (parameters.degradation_preference &&
      parameters.degradation_preference !=
          webrtc::DegradationPreference::BALANCED) {
    NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "degradationPreference",
                                          parameters.degradation_preference)
  }
  return Pure(scope.Escape(object));
}

static webrtc::RtpParameters NapiToRtpParameters(
    std::string const &transactionId,
    std::vector<webrtc::RtpHeaderExtensionParameters> const &headerExtensions,
    webrtc::RtcpParameters const &rtcp,
    std::vector<webrtc::RtpCodecParameters> const &codecs,
    std::vector<webrtc::RtpEncodingParameters> const &encodings,
    node_webrtc::Maybe<webrtc::DegradationPreference> degradationPreference) {
  webrtc::RtpParameters parameters;
  parameters.transaction_id = transactionId;
  parameters.header_extensions = headerExtensions;
  parameters.rtcp = rtcp;
  parameters.codecs = codecs;
  parameters.encodings = encodings;
  parameters.degradation_preference =
      degradationPreference
          .Map([](auto degradationPreference) {
            return absl::make_optional(
                static_cast<webrtc::DegradationPreference>(
                    degradationPreference));
          })
          .FromMaybe(absl::optional<webrtc::DegradationPreference>());
  return parameters;
}

FROM_NAPI_IMPL(webrtc::RtpParameters, value) {
  return From<Napi::Object>(value).FlatMap<webrtc::RtpParameters>(
      [](auto object) {
        return curry(NapiToRtpParameters) %
               GetRequired<std::string>(object, "transactionId") *
               GetRequired<std::vector<webrtc::RtpHeaderExtensionParameters>>(
                   object, "headerExtensions") *
               GetRequired<webrtc::RtcpParameters>(object, "rtcp") *
               GetRequired<std::vector<webrtc::RtpCodecParameters>>(object,
                                                                    "codecs") *
               GetRequired<std::vector<webrtc::RtpEncodingParameters>>(
                   object, "encodings") *
               GetOptional<webrtc::DegradationPreference>(
                   object, "degradationPreference");
      });
}

} // namespace node_webrtc
