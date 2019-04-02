#include "src/dictionaries/webrtc/rtp_parameters.h"

#include <nan.h>
#include <v8.h>
#include <webrtc/api/rtp_parameters.h>

#include "src/converters/v8.h"
#include "src/dictionaries/macros/napi.h"
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
  // NOTE(mroberts): Unsupported at this time.
  object->Set(Nan::New("encodings").ToLocalChecked(), Nan::New<v8::Array>());
  object->Set(Nan::New("rtcp").ToLocalChecked(), rtcp);
  return scope.Escape(object);
}

namespace napi {

static Validation<Napi::Value> CreateRtpParameters(Napi::Value headerExtensions, Napi::Value codecs, Napi::Value rtcp) {
  auto env = headerExtensions.Env();
  Napi::EscapableHandleScope scope(env);
  NODE_WEBRTC_CREATE_OBJECT_OR_RETURN(env, object)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "headerExtensions", headerExtensions)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "codecs", codecs)
  // NOTE(mroberts): Unsupported at this time.
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "encodings", std::vector<bool>())
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "rtcp", rtcp)
  return Pure(scope.Escape(object));
}

}  // namespace napi

TO_JS_IMPL(webrtc::RtpParameters, params) {
  return curry(CreateRtpParameters)
      % From<v8::Local<v8::Value>>(params.header_extensions)
      * From<v8::Local<v8::Value>>(params.codecs)
      * From<v8::Local<v8::Value>>(params.rtcp);
}

TO_NAPI_IMPL(webrtc::RtpParameters, pair) {
  return Validation<Napi::Value>::Join(curry(napi::CreateRtpParameters)
          % From<Napi::Value>(std::make_pair(pair.first, pair.second.header_extensions))
          * From<Napi::Value>(std::make_pair(pair.first, pair.second.codecs))
          * From<Napi::Value>(std::make_pair(pair.first, pair.second.rtcp)));
}

}  // namespace node_webrtc
