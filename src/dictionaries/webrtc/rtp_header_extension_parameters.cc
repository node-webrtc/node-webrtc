#include "src/dictionaries/webrtc/rtp_header_extension_parameters.h"

#include <utility>

#include <nan.h>
#include <node-addon-api/napi.h>
#include <v8.h>
#include <webrtc/api/rtp_parameters.h>

#include "src/dictionaries/macros/napi.h"
#include "src/functional/validation.h"

namespace node_webrtc {

TO_JS_IMPL(webrtc::RtpHeaderExtensionParameters, params) {
  Nan::EscapableHandleScope scope;
  auto object = Nan::New<v8::Object>();
  object->Set(Nan::New("uri").ToLocalChecked(), Nan::New(params.uri).ToLocalChecked());
  object->Set(Nan::New("id").ToLocalChecked(), Nan::New(params.id));
  return Pure(scope.Escape(object.As<v8::Value>()));
}

TO_NAPI_IMPL(webrtc::RtpHeaderExtensionParameters, pair) {
  auto env = pair.first;
  Napi::EscapableHandleScope scope(env);
  auto params = pair.second;
  NODE_WEBRTC_CREATE_OBJECT_OR_RETURN(env, object)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "uri", params.uri)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "id", params.id)
  return Pure(scope.Escape(object));
}

}  // namespace node_webrtc
