#pragma once

#include "src/converters/napi.h"
#include "src/functional/validation.h"  // IWYU pragma: keep

namespace node_webrtc {

#define CONVERT_INTERFACE_TO_NAPI(IFACE, NAME) \
  TO_NAPI_IMPL(IFACE*, pair) { \
    return Pure(pair.second->Value().As<Napi::Value>()); \
  }

#define CONVERT_INTERFACE_FROM_NAPI(IFACE, NAME) \
  FROM_NAPI_IMPL(IFACE*, value) { \
    return From<Napi::Object>(value).FlatMap<IFACE*>([](auto object) { \
      auto isInstance = false; \
      napi_instanceof(object.Env(), object, IFACE::constructor().Value(), &isInstance); \
      if (object.Env().IsExceptionPending()) { \
        return Validation<IFACE*>::Invalid(object.Env().GetAndClearPendingException().Message()); \
      } else if (!isInstance) { \
        return Validation<IFACE*>::Invalid("This is not an instance of " NAME); \
      } \
      return Pure(IFACE::Unwrap(object)); \
    }); \
  }

#define CONVERT_INTERFACE_TO_AND_FROM_NAPI(IFACE, NAME) \
  CONVERT_INTERFACE_TO_NAPI(IFACE, NAME) \
  CONVERT_INTERFACE_FROM_NAPI(IFACE, NAME)

}  // namespace node_webrtc
