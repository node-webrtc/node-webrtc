#pragma once

#include "src/converters/napi.hh"

namespace node_webrtc {

class Undefined {
public:
  Undefined() = default;
};

DECLARE_TO_NAPI(Undefined)

} // namespace node_webrtc
