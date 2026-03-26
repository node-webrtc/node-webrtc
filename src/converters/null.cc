#include "src/converters/null.hh"

#include <node-addon-api/napi.h>

#include "src/functional/validation.hh"

namespace node_webrtc {

FROM_NAPI_IMPL(Null, value) {
  return value.IsNull() ? Pure(Null())
                        : Validation<Null>::Invalid("Expected null");
}

} // namespace node_webrtc
