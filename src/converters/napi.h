#pragma once

#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>
#include <utility>

#include <node-addon-api/napi.h>

#include "src/converters.h"
#include "src/converters/macros.h"

namespace node_webrtc {

#define DECLARE_TO_NAPI(T) DECLARE_CONVERTER(std::pair<Napi::Env COMMA T>, Napi::Value)

#define DECLARE_FROM_NAPI(T) DECLARE_CONVERTER(Napi::Value, T)

#define DECLARE_TO_AND_FROM_NAPI(T) \
  DECLARE_TO_NAPI(T) \
  DECLARE_FROM_NAPI(T)

#define TO_NAPI_IMPL(T, V) CONVERTER_IMPL(std::pair<Napi::Env COMMA T>, Napi::Value, V)

#define FROM_NAPI_IMPL(T, V) CONVERTER_IMPL(Napi::Value, T, V)

DECLARE_TO_AND_FROM_NAPI(bool)
DECLARE_TO_AND_FROM_NAPI(double)
DECLARE_TO_AND_FROM_NAPI(uint8_t)
DECLARE_TO_AND_FROM_NAPI(uint16_t)
DECLARE_TO_AND_FROM_NAPI(uint32_t)
DECLARE_TO_AND_FROM_NAPI(uint64_t)
DECLARE_TO_AND_FROM_NAPI(int8_t)
DECLARE_TO_AND_FROM_NAPI(int16_t)
DECLARE_TO_AND_FROM_NAPI(int32_t)
DECLARE_TO_AND_FROM_NAPI(int64_t)
DECLARE_TO_AND_FROM_NAPI(std::string)
DECLARE_FROM_NAPI(Napi::Function)
DECLARE_FROM_NAPI(Napi::Object)
DECLARE_TO_NAPI(std::vector<bool>)
DECLARE_FROM_NAPI(Napi::ArrayBuffer)

}  // namespace node_webrtc
