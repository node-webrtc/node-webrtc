#include "src/converters/null.h"

#include <v8.h>

#include "src/functional/validation.h"

namespace node_webrtc {

FROM_JS_IMPL(Null, value) {
  return value->IsNull()
      ? Pure(Null())
      : Validation<Null>::Invalid("Expected null");
}

}  // namespace node_webrtc




