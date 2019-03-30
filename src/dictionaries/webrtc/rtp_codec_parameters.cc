#include "src/dictionaries/webrtc/rtp_codec_parameters.h"

#include <iosfwd>
#include <string>
#include <unordered_map>
#include <utility>

#include <absl/types/optional.h>
#include <nan.h>
#include <webrtc/api/rtp_parameters.h>
#include <v8.h>

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
    unsigned long i = 0;
    for (auto param : params.parameters) {
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

}  // namespace node_webrtc
