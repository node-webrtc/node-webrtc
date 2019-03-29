#include "src/dictionaries/webrtc/rtp_parameters.h"

#include <nan.h>
#include <webrtc/api/rtp_parameters.h>
#include <v8.h>

#include "src/converters/v8.h"
#include "src/dictionaries/webrtc/rtcp_parameters.h"
#include "src/dictionaries/webrtc/rtp_codec_parameters.h"
#include "src/dictionaries/webrtc/rtp_header_extension_parameters.h"
#include "src/functional/curry.h"
#include "src/functional/operators.h"
#include "src/functional/validation.h"

namespace node_webrtc {

static v8::Local<v8::Value> CreateRtpParameters(v8::Local<v8::Value> headerExtensions, v8::Local<v8::Value> codecs, v8::Local<v8::Value> rtcp) {
  Nan::EscapableHandleScope scope;
  auto object = Nan::New<v8::Object>();
  object->Set(Nan::New("headerExtensions").ToLocalChecked(), headerExtensions);
  object->Set(Nan::New("codecs").ToLocalChecked(), codecs);
  object->Set(Nan::New("encodings").ToLocalChecked(), Nan::New<v8::Array>());
  object->Set(Nan::New("rtcp").ToLocalChecked(), rtcp);
  return scope.Escape(object);
}

TO_JS_IMPL(webrtc::RtpParameters, params) {
  return curry(CreateRtpParameters)
      % From<v8::Local<v8::Value>>(params.header_extensions)
      * From<v8::Local<v8::Value>>(params.codecs)
      * From<v8::Local<v8::Value>>(params.rtcp);
}

}  // namespace node_webrtc
