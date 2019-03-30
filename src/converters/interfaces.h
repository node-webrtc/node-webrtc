#pragma once

#include <nan.h>  // IWYU pragma: keep
#include <v8.h>  // IWYU pragma: keep

#include "src/converters/v8.h"
#include "src/functional/validation.h"  // IWYU pragma: keep

namespace node_webrtc {

#define CONVERT_INTERFACE_TO_JS(IFACE, NAME, TO_FN) \
  TO_JS_IMPL(IFACE*, value) { \
    Nan::EscapableHandleScope scope; \
    if (!value) { \
      return Validation<v8::Local<v8::Value>>::Invalid(NAME " is null"); \
    } \
    return Pure(scope.Escape(value->TO_FN()).As<v8::Value>()); \
  }

// FIXME(mroberts): This is not safe.
#define CONVERT_INTERFACE_FROM_JS(IFACE, NAME, FROM_FN) \
  FROM_JS_IMPL(IFACE*, value) { \
    auto isolate = Nan::GetCurrentContext()->GetIsolate(); \
    auto tpl = IFACE::tpl().Get(isolate); \
    return tpl->HasInstance(value) \
        ? Pure(FROM_FN(value->ToObject())) \
        : Validation<IFACE*>::Invalid("This is not an instance of " NAME); \
  }

#define CONVERT_INTERFACE_TO_AND_FROM_JS(IFACE, NAME, TO_FN, FROM_FN) \
  CONVERT_INTERFACE_TO_JS(IFACE, NAME, TO_FN) \
  CONVERT_INTERFACE_FROM_JS(IFACE, NAME, FROM_FN)

} // namespace node_webrtc
