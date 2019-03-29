#pragma once

#include <absl/types/optional.h>
#include <nan.h>
#include <v8.h>

#include "src/converters.h"
#include "src/functional/validation.h"

namespace node_webrtc {

template <typename T>
struct Converter<absl::optional<T>, v8::Local<v8::Value>> {
  static Validation<v8::Local<v8::Value>> Convert(absl::optional<T> value) {
    if (value) {
      return Converter<T, v8::Local<v8::Value>>::Convert(*value);
    }
    return Pure(Nan::Null().As<v8::Value>());
  }
};

}  // namespace node_webrtc
