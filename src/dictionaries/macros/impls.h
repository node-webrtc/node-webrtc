#ifdef DICT

#include <node-addon-api/napi.h>
#include <v8.h>

#include "src/converters.h"
#include "src/converters/napi.h"
#include "src/converters/object.h"
#include "src/converters/v8.h"
#include "src/functional/curry.h"
#include "src/functional/operators.h"
#include "src/functional/validation.h"

namespace node_webrtc {

#define DICT_REQUIRED(type, memberName, stringValue) * GetRequired<type>(object, stringValue)
#define DICT_OPTIONAL(type, memberName, stringValue) * GetOptional<type>(object, stringValue)
#define DICT_DEFAULT(type, memberName, stringValue, defaultValue) * GetOptional<type>(object, stringValue, defaultValue)

FROM_JS_IMPL(DICT(), value) {
  return From<v8::Local<v8::Object>>(value).FlatMap<DICT()>([](auto object) {
    return Validation<DICT()>::Join(Pure(curry(DICT(_FN)))
            DICT(_LIST));
  });
}

#undef DICT_REQUIRED
#undef DICT_OPTIONAL
#undef DICT_DEFAULT

#define DICT_REQUIRED(type, memberName, stringValue) * napi::GetRequired<type>(object, stringValue)
#define DICT_OPTIONAL(type, memberName, stringValue) * napi::GetOptional<type>(object, stringValue)
#define DICT_DEFAULT(type, memberName, stringValue, defaultValue) * napi::GetOptional<type>(object, stringValue, defaultValue)

FROM_NAPI_IMPL(DICT(), value) {
  return From<Napi::Object>(value).FlatMap<DICT()>([](auto object) {
    return Validation<DICT()>::Join(Pure(curry(DICT(_FN)))
            DICT(_LIST));
  });
}

#undef DICT_REQUIRED
#undef DICT_OPTIONAL
#undef DICT_DEFAULT

}  // namespace node_webrtc

#endif  // DICT
