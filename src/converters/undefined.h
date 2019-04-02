#pragma once

#include "src/converters/napi.h"
#include "src/converters/v8.h"

namespace node_webrtc {

class Undefined {
 public:
  Undefined() = default;
};

DECLARE_TO_JS(Undefined)
DECLARE_TO_NAPI(Undefined)

}  // namespace node_webrtc
