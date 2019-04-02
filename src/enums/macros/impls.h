#ifdef ENUM

#include <utility>

#include <node-addon-api/napi.h>
#include <v8.h>

#include "src/converters.h"
#include "src/converters/napi.h"
#include "src/converters/v8.h"

namespace node_webrtc {

#define ENUM_SUPPORTED(VALUE, STRING) \
  case VALUE: \
  return Pure<std::string>(STRING);

#define ENUM_UNSUPPORTED(VALUE, STRING, ERROR) \
  case VALUE: \
  return Validation<std::string>::Invalid(ERROR);

CONVERTER_IMPL(ENUM(), std::string, value) {
  switch (value) {
      ENUM(_LIST)
  }
}

CONVERT_VIA(v8::Local<v8::Value>, std::string, ENUM())
CONVERT_VIA(Napi::Value, std::string, ENUM())

#undef ENUM_SUPPORTED
#undef ENUM_UNSUPPORTED

#define ENUM_SUPPORTED(VALUE, STRING) \
  if (value == STRING) { \
    return Pure(VALUE); \
  }

#define ENUM_UNSUPPORTED(VALUE, STRING, ERROR) \
  if (value == STRING) { \
    return Validation<decltype(VALUE)>::Invalid(ERROR); \
  }

CONVERTER_IMPL(std::string, ENUM(), value) {
  ENUM(_LIST)
  return Validation<ENUM()>::Invalid("Invalid " ENUM(_NAME));
}

CONVERT_VIA(ENUM(), std::string, v8::Local<v8::Value>)

TO_NAPI_IMPL(ENUM(), pair) {
  return From<std::string>(pair.second).FlatMap<Napi::Value>([env = pair.first](auto value) {
    return From<Napi::Value>(std::make_pair(env, value));
  });
}

#undef ENUM_SUPPORTED
#undef ENUM_UNSUPPORTED

}  // namespace node_webrtc

#endif  // ENUM
