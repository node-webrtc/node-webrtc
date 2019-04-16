#pragma once

#include <utility>

#include <absl/types/optional.h>
#include <node-addon-api/napi.h>

#include "src/converters.h"
#include "src/functional/validation.h"

namespace node_webrtc {

template <typename T>
struct Converter<std::pair<Napi::Env, absl::optional<T>>, Napi::Value> {
  static Validation<Napi::Value> Convert(std::pair<Napi::Env, absl::optional<T>> pair) {
    return pair.second
        ? From<Napi::Value>(std::make_pair(pair.first, *pair.second))
        : Pure(pair.first.Null());
  }
};

}  // namespace node_webrtc
