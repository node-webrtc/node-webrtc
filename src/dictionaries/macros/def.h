#ifdef DICT

#include "src/functional/maybe.h"

#define DICT_REQUIRED(TYPE, VAR, PROP) TYPE VAR;
#define DICT_OPTIONAL(TYPE, VAR, PROP) Maybe<TYPE> VAR;
#define DICT_DEFAULT(TYPE, VAR, PROP, DEFAULT) TYPE VAR;

namespace node_webrtc {

struct DICT() {
  DICT(_LIST)
};

#undef DICT_REQUIRED
#undef DICT_OPTIONAL
#undef DICT_DEFAULT

}  // namespace webrtc

#endif  // DICT
