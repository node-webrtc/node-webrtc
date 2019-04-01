#pragma once

#include "src/converters/v8.h"

namespace node_webrtc {

class Undefined {
 public:
  Undefined() = default;
};

DECLARE_TO_JS(Undefined)

}  // namespace node_webrtc
