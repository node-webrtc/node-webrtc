#ifdef DICT

#include <v8.h>

#include "src/converters.h"
#include "src/converters/object.h"
#include "src/converters/v8.h"
#include "src/functional/curry.h"
#include "src/functional/operators.h"
#include "src/functional/validation.h"

namespace node_webrtc {

#define REQUIRED(type, memberName, stringValue) * GetRequired<type>(object, stringValue)
#define OPTIONAL(type, memberName, stringValue) * GetOptional<type>(object, stringValue)
#define DEFAULT(type, memberName, stringValue, defaultValue) * GetOptional<type>(object, stringValue, defaultValue)

FROM_JS_IMPL(DICT(), value) {
  return From<v8::Local<v8::Object>>(value).FlatMap<DICT()>([](auto object) {
    return Validation<DICT()>::Join(Pure(curry(DICT(_FN)))
            DICT(_LIST));
  });
}

#undef REQUIRED
#undef OPTIONAL
#undef DEFAULT

}  // namespace node_webrtc

#endif  // DICT
