#include "src/dictionaries/webrtc/rtp_codec_parameters.h"

#include <cstdint>
#include <iosfwd>
#include <string>
#include <unordered_map>
#include <utility>

#include <absl/types/optional.h>
#include <nan.h>
#include <node-addon-api/napi.h>
#include <v8.h>
#include <webrtc/api/rtp_parameters.h>

#include "src/dictionaries/macros/napi.h"
#include "src/functional/validation.h"

namespace node_webrtc {

TO_JS_IMPL(webrtc::RtpCodecParameters, params) {
  Nan::EscapableHandleScope scope;
  auto object = Nan::New<v8::Object>();
  object->Set(Nan::New("payloadType").ToLocalChecked(), Nan::New(params.payload_type));
  object->Set(Nan::New("mimeType").ToLocalChecked(), Nan::New(params.mime_type()).ToLocalChecked());
  if (params.clock_rate) {
    object->Set(Nan::New("clockRate").ToLocalChecked(), Nan::New(*params.clock_rate));
  }
  if (params.num_channels) {
    object->Set(Nan::New("channels").ToLocalChecked(), Nan::New(*params.num_channels));
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
    object->Set(Nan::New("sdpFmtpLine").ToLocalChecked(), Nan::New(fmtp).ToLocalChecked());
  }
  return Pure(scope.Escape(object).As<v8::Value>());
}

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

}  // namespace node_webrtc
