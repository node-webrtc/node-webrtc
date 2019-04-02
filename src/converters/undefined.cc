#include "src/converters/undefined.h"

#include <utility>

#include <nan.h>
#include <node-addon-api/napi.h>
#include <v8.h>

#include "src/functional/validation.h"

namespace node_webrtc {

TO_JS_IMPL(Undefined, value) {
  (void) value;
  Nan::EscapableHandleScope scope;
  return Pure(scope.Escape(Nan::Undefined().As<v8::Value>()));
}

TO_NAPI_IMPL(Undefined, pair) {
  Napi::EscapableHandleScope scope(pair.first);
  return Pure(scope.Escape(pair.first.Undefined()));
}

}  // namespace node_webrtc
