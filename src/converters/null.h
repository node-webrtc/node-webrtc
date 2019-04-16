#pragma once

#include "src/converters/napi.h"

namespace node_webrtc {

class Null {
 public:
  Null() = default;
};

DECLARE_FROM_NAPI(Null)

}  // namespace node_webrtc
