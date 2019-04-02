#include "src/dictionaries/node_webrtc/unsigned_short_range.h"

#include <utility>

#include <nan.h>
#include <node-addon-api/napi.h>
#include <v8.h>

#include "src/converters/napi.h"
#include "src/converters/v8.h"
#include "src/dictionaries/macros/napi.h"
#include "src/functional/maybe.h"
#include "src/functional/validation.h"

namespace node_webrtc {

#define UNSIGNED_SHORT_RANGE_FN CreateUnsignedShortRange

static Validation<UNSIGNED_SHORT_RANGE> UNSIGNED_SHORT_RANGE_FN(
    const Maybe<uint16_t>& maybeMin,
    const Maybe<uint16_t>& maybeMax) {
  auto min = maybeMin.FromMaybe(0);
  auto max = maybeMax.FromMaybe(65535);
  if (min > max) {
    return Validation<UNSIGNED_SHORT_RANGE>::Invalid("Expected min to be less than max");
  }
  return Pure<UNSIGNED_SHORT_RANGE>({maybeMin, maybeMax});
}

TO_JS_IMPL(UNSIGNED_SHORT_RANGE, value) {
  Nan::EscapableHandleScope scope;
  auto object = Nan::New<v8::Object>();
  if (value.min.IsJust()) {
    object->Set(Nan::New("min").ToLocalChecked(), Nan::New(value.min.UnsafeFromJust()));
  }
  if (value.max.IsJust()) {
    object->Set(Nan::New("max").ToLocalChecked(), Nan::New(value.max.UnsafeFromJust()));
  }
  return Pure(scope.Escape(object.As<v8::Value>()));
}

TO_NAPI_IMPL(UNSIGNED_SHORT_RANGE, pair) {
  auto env = pair.first;
  Napi::EscapableHandleScope scope(pair.first);

  NODE_WEBRTC_CREATE_OBJECT_OR_RETURN(env, object)

  auto value = pair.second;
  if (value.min.IsJust()) {
    NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "min", value.min.UnsafeFromJust())
  }
  if (value.max.IsJust()) {
    NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "max", value.max.UnsafeFromJust())
  }

  return Pure(scope.Escape(object));
}

}  // namespace node_webrtc

#define DICT(X) UNSIGNED_SHORT_RANGE ## X
#include "src/dictionaries/macros/impls.h"
#undef DICT
