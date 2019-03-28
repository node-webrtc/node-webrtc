#ifdef ENUM

#include <v8.h>

#include "src/converters.h"
#include "src/converters/v8.h"

namespace node_webrtc {

#define ENUM_TO_STRING_SUPPORTED(ENUM, VALUE, STRING) \
  case ENUM::VALUE: \
  return node_webrtc::Pure<std::string>(STRING);

#define ENUM_TO_STRING_UNSUPPORTED(ENUM, VALUE, ERROR) \
  case ENUM::VALUE: \
  return node_webrtc::Validation<std::string>::Invalid(ERROR);

#define SUPPORTED(ENUM, VALUE, STRING) ENUM_TO_STRING_SUPPORTED(ENUM, VALUE, STRING)
#define UNSUPPORTED(ENUM, VALUE, STRING, ERROR) ENUM_TO_STRING_UNSUPPORTED(ENUM, VALUE, ERROR)

CONVERTER_IMPL(ENUM(), std::string, value) {
  switch (value) {
      ENUM(_LIST)
  }
}

CONVERT_VIA(v8::Local<v8::Value>, std::string, ENUM())

#undef ENUM_TO_STRING
#undef ENUM_TO_STRING_SUPPORTED
#undef ENUM_TO_STRING_UNSUPPORTED
#undef SUPPORTED
#undef UNSUPPORTED

#define STRING_TO_ENUM_SUPPORTED(ENUM, VALUE, STRING) \
  if (value == STRING) { \
    return node_webrtc::Pure(ENUM::VALUE); \
  }

#define STRING_TO_ENUM_UNSUPPORTED(ENUM, STRING, ERROR) \
  if (value == STRING) { \
    return node_webrtc::Validation<ENUM>::Invalid(ERROR); \
  }

#define SUPPORTED(ENUM, VALUE, STRING) STRING_TO_ENUM_SUPPORTED(ENUM, VALUE, STRING)
#define UNSUPPORTED(ENUM, VALUE, STRING, ERROR) STRING_TO_ENUM_UNSUPPORTED(ENUM, STRING, ERROR)

CONVERTER_IMPL(std::string, ENUM(), value) {
  ENUM(_LIST)
  return node_webrtc::Validation<ENUM()>::Invalid("Invalid " ENUM(_NAME));
}

CONVERT_VIA(ENUM(), std::string, v8::Local<v8::Value>)

#undef STRING_TO_ENUM
#undef STRING_TO_ENUM_SUPPORTED
#undef STRING_TO_ENUM_UNSUPPORTED
#undef SUPPORTED
#undef UNSUPPORTED

}  // namespace node_webrtc

#endif  // ENUM
