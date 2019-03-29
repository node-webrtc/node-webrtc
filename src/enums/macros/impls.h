#ifdef ENUM

#include <v8.h>

#include "src/converters.h"
#include "src/converters/v8.h"

namespace node_webrtc {

#define SUPPORTED(ENUM, VALUE, STRING) \
  case ENUM::VALUE: \
  return Pure<std::string>(STRING);

#define UNSUPPORTED(ENUM, VALUE, STRING, ERROR) \
  case ENUM::VALUE: \
  return Validation<std::string>::Invalid(ERROR);

CONVERTER_IMPL(ENUM(), std::string, value) {
  switch (value) {
      ENUM(_LIST)
  }
}

CONVERT_VIA(v8::Local<v8::Value>, std::string, ENUM())

#undef SUPPORTED
#undef UNSUPPORTED

#define SUPPORTED(ENUM, VALUE, STRING) \
  if (value == STRING) { \
    return Pure(ENUM::VALUE); \
  }

#define UNSUPPORTED(ENUM, VALUE, STRING, ERROR) \
  if (value == STRING) { \
    return Validation<ENUM>::Invalid(ERROR); \
  }

CONVERTER_IMPL(std::string, ENUM(), value) {
  ENUM(_LIST)
  return Validation<ENUM()>::Invalid("Invalid " ENUM(_NAME));
}

CONVERT_VIA(ENUM(), std::string, v8::Local<v8::Value>)

#undef SUPPORTED
#undef UNSUPPORTED

}  // namespace node_webrtc

#endif  // ENUM
