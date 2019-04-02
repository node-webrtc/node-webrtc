#include "src/dictionaries/webrtc/rtcp_parameters.h"

#include <iosfwd>
#include <string>
#include <utility>

#include <nan.h>
#include <node-addon-api/napi.h>
#include <v8.h>
#include <webrtc/api/rtp_parameters.h>

#include "src/dictionaries/macros/napi.h"
#include "src/functional/validation.h"

namespace node_webrtc {

TO_JS_IMPL(webrtc::RtcpParameters, params) {
  Nan::EscapableHandleScope scope;
  auto object = Nan::New<v8::Object>();
  if (!params.cname.empty()) {
    object->Set(Nan::New("cname").ToLocalChecked(), Nan::New(params.cname).ToLocalChecked());
  }
  object->Set(Nan::New("reducedSize").ToLocalChecked(), Nan::New(params.reduced_size));
  return Pure(scope.Escape(object.As<v8::Value>()));
}

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

}  // namespace node_webrtc
