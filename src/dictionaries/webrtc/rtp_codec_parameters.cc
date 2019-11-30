#include "src/dictionaries/webrtc/rtp_codec_parameters.h"

#include <cstdint>
#include <iosfwd>
#include <string>
#include <unordered_map>
#include <utility>

#include <absl/types/optional.h>
#include <node-addon-api/napi.h>
#include <webrtc/api/rtp_parameters.h>

#include "src/converters.h"
#include "src/converters/object.h"
#include "src/dictionaries/macros/napi.h"
#include "src/functional/curry.h"
#include "src/functional/maybe.h"
#include "src/functional/operators.h"
#include "src/functional/validation.h"

namespace node_webrtc {

TO_NAPI_IMPL(webrtc::RtpCodecParameters, pair) {
  auto env = pair.first;
  Napi::EscapableHandleScope scope(env);
  auto params = pair.second;
  NODE_WEBRTC_CREATE_OBJECT_OR_RETURN(env, object)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "payloadType", params.payload_type)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "mimeType", params.mime_type())
  if (params.clock_rate) {
    NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "clockRate", *params.clock_rate)
  }
  if (params.num_channels) {
    NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "channels", *params.num_channels)
  }
  if (!params.parameters.empty()) {
    std::string fmtp("a=fmtp:" + std::to_string(params.payload_type));
    uint64_t i = 0;
    for (const auto& param : params.parameters) {
      fmtp += " " + param.first + "=" + param.second;
      if (i < params.parameters.size() - 1) {
        fmtp += ";";
      }
      i++;
    }
    NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "sdpFmtpLine", fmtp)
  }
  return Pure(scope.Escape(object));
}

static webrtc::RtpCodecParameters NapiToRtpCodecParameters(
    const uint8_t payloadType,
    const std::string mimeType,
    const uint64_t clockRate,
    node_webrtc::Maybe<uint8_t> channels,
    node_webrtc::Maybe<std::string> maybeSdpFmtpLine) {
  webrtc::RtpCodecParameters result;
  auto indexOfSlash = mimeType.find("/");
  auto kindString = mimeType.substr(0, indexOfSlash);
  auto nameString = mimeType.substr(indexOfSlash + 1);
  result.kind = kindString == "audio" ? cricket::MEDIA_TYPE_AUDIO : cricket::MEDIA_TYPE_VIDEO;
  result.name = nameString;
  result.payload_type = payloadType;
  result.clock_rate = clockRate;
  if (channels.IsJust()) {
    result.num_channels = channels.UnsafeFromJust();
  }
  if (maybeSdpFmtpLine.IsJust()) {
    auto sdpFmtpLine = maybeSdpFmtpLine.UnsafeFromJust() + ";";
    size_t pos = 0;
    std::string keyValue;
    while ((pos = sdpFmtpLine.find(";")) != std::string::npos) {
      keyValue = sdpFmtpLine.substr(0, pos);
      sdpFmtpLine.erase(0, pos + 1);
      auto indexOfEquals = keyValue.find("=");
      auto key = keyValue.substr(0, indexOfEquals);
      auto value = keyValue.substr(indexOfEquals + 1);
      result.parameters[key] = value;
    }
  }
  return result;
}

FROM_NAPI_IMPL(webrtc::RtpCodecParameters, value) {
  return From<Napi::Object>(value).FlatMap<webrtc::RtpCodecParameters>([](auto object) {
    return curry(NapiToRtpCodecParameters)
        % GetRequired<uint8_t>(object, "payloadType")
        * GetRequired<std::string>(object, "mimeType")
        * GetRequired<uint64_t>(object, "clockRate")
        * GetOptional<uint8_t>(object, "channels")
        * GetOptional<std::string>(object, "sdpFmtpLine");
  });
}

}  // namespace node_webrtc
