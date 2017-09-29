/* Copyright (c) 2017 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */

/*
 * This file defines conversion functions between native and v8 data types.
 */

#ifndef SRC_CONVERTERS_V8_H_
#define SRC_CONVERTERS_V8_H_

#include <nan.h>

#include "src/converters.h"
#include "src/functional/validation.h"

namespace node_webrtc {

// TODO(mroberts): The following could probably all be moved into a v8.cc file.

template <>
struct Converter<v8::Local<v8::Value>, bool> {
  static Validation<bool> Convert(const v8::Local<v8::Value> value) {
    auto maybeBoolean = Nan::To<v8::Boolean>(value);
    if (maybeBoolean.IsEmpty()) {
      return Validation<bool>::Invalid("Expected a bool");
    }
    auto boolean = (*maybeBoolean.ToLocalChecked())->Value();
    return Validation<bool>::Valid(boolean);
  }
};

template <>
struct Converter<v8::Local<v8::Value>, int32_t> {
  static Validation<int32_t> Convert(const v8::Local<v8::Value> value) {
    auto maybeInt32 = Nan::To<v8::Int32>(value);
    if (maybeInt32.IsEmpty()) {
      return Validation<int32_t>::Invalid("Expected a 32-bit integer");
    }
    auto int32 = (*maybeInt32.ToLocalChecked())->Value();
    return Validation<int32_t>::Valid(int32);
  }
};

template <>
struct Converter<v8::Local<v8::Value>, int64_t> {
  static Validation<int64_t> Convert(const v8::Local<v8::Value> value) {
    auto maybeInteger = Nan::To<v8::Integer>(value);
    if (maybeInteger.IsEmpty()) {
      return Validation<int64_t>::Invalid("Expected a 64-bit integer");
    }
    auto integer = (*maybeInteger.ToLocalChecked())->Value();
    return Validation<int64_t>::Valid(integer);
  }
};

template <>
struct Converter<v8::Local<v8::Value>, double> {
  static Validation<double> Convert(const v8::Local<v8::Value> value) {
    auto maybeNumber = Nan::To<v8::Number>(value);
    if (maybeNumber.IsEmpty()) {
      return Validation<double>::Invalid("Expected a double");
    }
    auto number = (*maybeNumber.ToLocalChecked())->Value();
    return Validation<double>::Valid(number);
  }
};

template <>
struct Converter<v8::Local<v8::Value>, std::string> {
  static Validation<std::string> Convert(const v8::Local<v8::Value> value) {
    if (!value->IsString()) {
      return Validation<std::string>::Invalid("Expected a string");
    }
    auto string = *v8::String::Utf8Value(value.As<v8::String>());
    return Validation<std::string>::Valid(string);
  }
};

template <typename T>
struct Converter<v8::Local<v8::Value>, std::vector<T>> {
  static Validation<std::vector<T>> Convert(const v8::Local<v8::Value> value) {
    if (!value->IsArray()) {
      return Validation<std::vector<T>>::Invalid("Expected an array");
    }
    auto array = value.As<v8::Array>();
    auto validated = std::vector<Validation<T>>();
    for (uint32_t i = 0; i < array->Length(); i++) {
      validated.push_back(From<T>(array->Get(i)));
    }
    return Validation<T>::Sequence(validated);
  }
};

template <>
struct Converter<v8::Local<v8::Value>, uint32_t> {
  static Validation<uint32_t> Convert(const v8::Local<v8::Value> value) {
    auto maybeUint32 = Nan::To<v8::Uint32>(value);
    if (maybeUint32.IsEmpty()) {
      return Validation<uint32_t>::Invalid("Expected a 32-bit unsigned integer");
    }
    auto uint32 = (*maybeUint32.ToLocalChecked())->Value();
    return Validation<uint32_t>::Valid(uint32);
  }
};

template <>
struct Converter<v8::Local<v8::Value>, v8::Local<v8::Object>> {
  static Validation<v8::Local<v8::Object>> Convert(const v8::Local<v8::Value> value) {
    Nan::EscapableHandleScope scope;
    auto maybeObject = Nan::To<v8::Object>(value);
    if (maybeObject.IsEmpty()) {
      return Validation<v8::Local<v8::Object>>::Invalid("Expected an object");
    }
    auto object = maybeObject.ToLocalChecked();
    return Validation<v8::Local<v8::Object>>::Valid(scope.Escape(object));
  }
};

}  // namespace node_webrtc

#endif  // SRC_CONVERTERS_V8_H_
