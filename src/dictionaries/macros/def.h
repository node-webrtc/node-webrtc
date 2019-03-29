#ifdef DICT

#include "src/functional/maybe.h"

#define REQUIRED(TYPE, VAR, PROP) TYPE VAR;
#define OPTIONAL(TYPE, VAR, PROP) Maybe<TYPE> VAR;
#define DEFAULT(TYPE, VAR, PROP, DEFAULT) TYPE VAR;

namespace node_webrtc {

struct DICT() {
  DICT(_LIST)
};

#undef REQUIRED
#undef OPTIONAL
#undef DEFAULT

}  // namespace webrtc

#endif  // DICT
