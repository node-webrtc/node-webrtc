#include "src/dictionaries/webrtc/rtp_header_extension_parameters.h"

#include <nan.h>
#include <webrtc/api/rtp_parameters.h>
#include <v8.h>

#include "src/functional/validation.h"

namespace node_webrtc {

TO_JS_IMPL(webrtc::RtpHeaderExtensionParameters, params) {
  Nan::EscapableHandleScope scope;
  auto object = Nan::New<v8::Object>();
  object->Set(Nan::New("uri").ToLocalChecked(), Nan::New(params.uri).ToLocalChecked());
  object->Set(Nan::New("id").ToLocalChecked(), Nan::New(params.id));
  return Pure(scope.Escape(object.As<v8::Value>()));
}

}  // namespace node_webrtc
