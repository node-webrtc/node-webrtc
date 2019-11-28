#include "src/dictionaries/webrtc/rtp_codec_capability.h"

#include <cstdint>
#include <iosfwd>
#include <string>

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

TO_NAPI_IMPL(webrtc::RtpCodecCapability, pair) {
  auto env = pair.first;
  Napi::EscapableHandleScope scope(env);
  auto capabilities = pair.second;
  NODE_WEBRTC_CREATE_OBJECT_OR_RETURN(env, object)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "mimeType", capabilities.mime_type())
  if (capabilities.clock_rate) {
    NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "clockRate", *capabilities.clock_rate)
  }
  if (capabilities.num_channels) {
    NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "channels", *capabilities.num_channels)
  }
  if (!capabilities.parameters.empty()) {
    std::string fmtp;
    uint64_t i = 0;
    for (const auto& param : capabilities.parameters) {
      fmtp += param.first + "=" + param.second;
      if (i < capabilities.parameters.size() - 1) {
        fmtp += ";";
      }
      i++;
    }
    NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "sdpFmtpLine", fmtp)
  }
  return Pure(scope.Escape(object));
}

static webrtc::RtpCodecCapability CreateRtpCodecCapability(
    std::string mimeType,
    uint64_t clockRate,
    Maybe<uint8_t> channels,
    Maybe<std::string> maybeSdpFmtpLine) {
  webrtc::RtpCodecCapability result;
  auto indexOfSlash = mimeType.find("/");
  auto kindString = mimeType.substr(0, indexOfSlash);
  auto nameString = mimeType.substr(indexOfSlash + 1);
  result.kind = kindString == "audio" ? cricket::MEDIA_TYPE_AUDIO : cricket::MEDIA_TYPE_VIDEO;
  result.name = nameString;
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

FROM_NAPI_IMPL(webrtc::RtpCodecCapability, value) {
  return From<Napi::Object>(value).FlatMap<webrtc::RtpCodecCapability>([](auto object) {
    return curry(CreateRtpCodecCapability)
        % GetRequired<std::string>(object, "mimeType")
        * GetRequired<uint64_t>(object, "clockRate")
        * GetOptional<uint8_t>(object, "channels")
        * GetOptional<std::string>(object, "sdpFmtpLine");
  });
}

}  // namespace node_webrtc
