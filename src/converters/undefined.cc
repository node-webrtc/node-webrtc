#include "src/converters/undefined.h"

#include <nan.h>
#include <v8.h>

#include "src/functional/validation.h"

namespace node_webrtc {

TO_JS_IMPL(Undefined, value) {
  (void) value;
  Nan::EscapableHandleScope scope;
  return Pure(scope.Escape(Nan::Undefined().As<v8::Value>()));
}

}  // namepsace node_webrtc
