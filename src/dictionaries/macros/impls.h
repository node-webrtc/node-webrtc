#ifdef DICT

#include <node-addon-api/napi.h>

#include "src/converters.h"
#include "src/converters/napi.h"
#include "src/converters/object.h"
#include "src/functional/curry.h"
#include "src/functional/operators.h"
#include "src/functional/validation.h"

namespace node_webrtc {

#define DICT_REQUIRED(type, memberName, stringValue) * GetRequired<type>(object, stringValue)
#define DICT_OPTIONAL(type, memberName, stringValue) * GetOptional<type>(object, stringValue)
#define DICT_DEFAULT(type, memberName, stringValue, defaultValue) * GetOptional<type>(object, stringValue, defaultValue)

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
