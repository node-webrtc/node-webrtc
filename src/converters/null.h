#pragma once

#include "src/converters/v8.h"

namespace node_webrtc {

class Null {
 public:
  Null() = default;
};

DECLARE_FROM_JS(Null)

}  // namespace node_webrtc
