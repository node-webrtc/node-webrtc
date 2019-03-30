/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */

/*
 * This file defines conversion functions between native and v8 data types.
 */

#pragma once

#include <cstdint>
#include <iosfwd>
#include <vector>

#include <nan.h>
#include <v8.h>

#include "src/converters.h"
#include "src/functional/maybe.h"
#include "src/functional/validation.h"

namespace node_webrtc {

// TODO(mroberts): The following could probably all be moved into a v8.cc file.

class Null {
 public:
  Null() = default;
};

DECLARE_FROM_JS(Null)

class Undefined {
 public:
  Undefined() = default;
};

DECLARE_TO_JS(Undefined)

DECLARE_TO_AND_FROM_JS(bool)
DECLARE_TO_AND_FROM_JS(double)
DECLARE_TO_AND_FROM_JS(int32_t)
DECLARE_TO_AND_FROM_JS(int64_t)
DECLARE_TO_AND_FROM_JS(uint8_t)
DECLARE_TO_AND_FROM_JS(uint16_t)
DECLARE_TO_AND_FROM_JS(uint32_t)
DECLARE_TO_JS(uint64_t)
DECLARE_TO_AND_FROM_JS(std::string)
DECLARE_FROM_JS(v8::Local<v8::External>)
DECLARE_FROM_JS(v8::Local<v8::Function>)
DECLARE_FROM_JS(v8::Local<v8::Object>)
DECLARE_TO_JS(v8::Local<v8::Primitive>)
DECLARE_TO_JS(std::vector<bool>)
DECLARE_FROM_JS(v8::ArrayBuffer::Contents)

template <typename T>
struct Converter<v8::Local<v8::Value>, Maybe<T>> {
  static Validation<Maybe<T>> Convert(const v8::Local<v8::Value> value) {
    if (value.IsEmpty() || value->IsUndefined()) {
      return Pure(Maybe<T>::Nothing());
    }
    return From<T>(value).Map(&Maybe<T>::Just);
  }
};

template <typename T>
struct Converter<Maybe<T>, v8::Local<v8::Value>> {
  static Validation<v8::Local<v8::Value>> Convert(const Maybe<T> value) {
    return value.IsJust()
        ? From<v8::Local<v8::Value>>(value.UnsafeFromJust())
        : Pure(Nan::Null().As<v8::Value>());
  }
};

template <>
struct Converter<v8::Local<v8::Value>, v8::Local<v8::Array>> {
  static Validation<v8::Local<v8::Array>> Convert(v8::Local<v8::Value> value) {
    return value->IsArray()
        ? Validation<v8::Local<v8::Array>>::Valid(value.As<v8::Array>())
        : Validation<v8::Local<v8::Array>>::Invalid("Expected an array");
  }
};

template <typename T>
struct Converter<v8::Local<v8::Array>, std::vector<T>> {
  static Validation<std::vector<T>> Convert(const v8::Local<v8::Array> array) {
    auto validated = std::vector<Validation<T>>();
    for (uint32_t i = 0; i < array->Length(); i++) {
      validated.push_back(From<T>(array->Get(i)));
    }
    return Validation<T>::Sequence(validated);
  }
};

template <typename T>
struct Converter<v8::Local<v8::Value>, std::vector<T>> {
  static Validation<std::vector<T>> Convert(const v8::Local<v8::Value> value) {
    return Converter<v8::Local<v8::Value>, v8::Local<v8::Array>>::Convert(value).FlatMap<std::vector<T>>(Converter<v8::Local<v8::Array>, std::vector<T>>::Convert);
  }
};

template <typename T>
struct Converter<std::vector<T>, v8::Local<v8::Value>> {
  static Validation<v8::Local<v8::Value>> Convert(const std::vector<T>& values) {
    Nan::EscapableHandleScope scope;
    auto array = Nan::New<v8::Array>();
    uint32_t i = 0;
    for (auto value : values) {
      auto maybeValue = From<v8::Local<v8::Value>>(value);
      if (maybeValue.IsInvalid()) {
        return Validation<v8::Local<v8::Value>>::Invalid(maybeValue.ToErrors());
      }
      array->Set(i++, maybeValue.UnsafeFromValid());
    }
    return Pure(scope.Escape(array).As<v8::Value>());
  }
};

}  // namespace node_webrtc
