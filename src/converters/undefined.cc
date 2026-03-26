#include "src/converters/undefined.hh"

#include <utility>

#include <node-addon-api/napi.h>

#include "src/functional/validation.hh"

namespace node_webrtc {

TO_NAPI_IMPL(Undefined, pair) {
  Napi::EscapableHandleScope scope(pair.first);
  return Pure(scope.Escape(pair.first.Undefined()));
}

} // namespace node_webrtc
