#include "src/dictionaries/webrtc/rtcp_parameters.h"

#include <iosfwd>
#include <string>

#include <nan.h>
#include <webrtc/api/rtp_parameters.h>
#include <v8.h>

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

}  // namespace node_webrtc
