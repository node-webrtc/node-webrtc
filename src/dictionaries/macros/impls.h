#ifdef DICT

#include <v8.h>

#include "src/converters.h"
#include "src/converters/object.h"
#include "src/converters/v8.h"
#include "src/functional/curry.h"
#include "src/functional/operators.h"
#include "src/functional/validation.h"

namespace node_webrtc {

#define OBJ_FROM_JS_IMPL1(TYPE, FN) \

#define EXPAND_OBJ_FROM_JS_DEFAULT(TYPE, NAME, DEFAULT) * GetOptional<TYPE>(object, NAME, DEFAULT)

#define EXPAND_OBJ_FROM_JS_OPTIONAL(TYPE, NAME) * GetOptional<TYPE>(object, NAME)

#define EXPAND_OBJ_FROM_JS_REQUIRED(TYPE, NAME) * GetRequired<TYPE>(object, NAME)

#define REQUIRED(type, memberName, stringValue) EXPAND_OBJ_FROM_JS_REQUIRED(type, stringValue)
#define OPTIONAL(type, memberName, stringValue) EXPAND_OBJ_FROM_JS_OPTIONAL(type, stringValue)
#define DEFAULT(type, memberName, stringValue, defaultValue) EXPAND_OBJ_FROM_JS_DEFAULT(type, stringValue, defaultValue)

FROM_JS_IMPL(DICT(), value) {
  return From<v8::Local<v8::Object>>(value).FlatMap<DICT()>(
  [](const v8::Local<v8::Object> object) {
    return Pure(curry(DICT(_FN)))
        DICT(_LIST);
  });
}


#undef OBJ_FROM_JS_IMPL1
#undef EXPAND_OBJ_FROM_JS_DEFAULT
#undef EXPAND_OBJ_FROM_JS_OPTIONAL
#undef EXPAND_OBJ_FROM_JS_REQUIRED
#undef REQUIRED
#undef OPTIONAL
#undef DEFAULT

}  // namespace node_webrtc

#endif  // DICT
