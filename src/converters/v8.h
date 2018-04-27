/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
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

#include "nan.h"

#include "src/converters.h"
#include "src/functional/validation.h"

namespace node_webrtc {

// TODO(mroberts): The following could probably all be moved into a v8.cc file.

class SomeError {
 public:
  SomeError() {}

  explicit SomeError(const std::string& message): _message(message) {}

  std::string message() const {
    return _message;
  }

 private:
  std::string _message;
};

template <>
struct Converter<SomeError, v8::Local<v8::Value>> {
  static Validation<v8::Local<v8::Value>> Convert(const SomeError someError) {
    Nan::EscapableHandleScope scope;
    auto error = static_cast<v8::Local<v8::Value>>(Nan::Error(Nan::New(someError.message()).ToLocalChecked()));
    return Validation<v8::Local<v8::Value>>::Valid(scope.Escape(error));
  }
};

class Null {
 public:
  Null() {}
};

template <>
struct Converter<v8::Local<v8::Value>, Null> {
  static Validation<Null> Convert(const v8::Local<v8::Value> value) {
    return value->IsNull()
        ? Validation<Null>::Valid(Null())
        : Validation<Null>::Invalid("Expected null");
  }
};

class Undefined {
 public:
  Undefined() {}
};

template <>
struct Converter<Undefined, v8::Local<v8::Value>> {
  static Validation<v8::Local<v8::Value>> Convert(const Undefined) {
    Nan::EscapableHandleScope scope;
    auto undefined = static_cast<v8::Local<v8::Value>>(Nan::Undefined());
    return Validation<v8::Local<v8::Value>>::Valid(scope.Escape(undefined));
  }
};

template <typename T>
struct Converter<v8::Local<v8::Value>, Maybe<T>> {
  static Validation<Maybe<T>> Convert(const v8::Local<v8::Value> value) {
    if (value.IsEmpty() || value->IsUndefined()) {
      return Validation<Maybe<T>>::Valid(Maybe<T>::Nothing());
    }
    return From<T>(value).Map(&Maybe<T>::Just);
  }
};

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
struct Converter<v8::Local<v8::Value>, uint8_t> {
  static Validation<uint8_t> Convert(const v8::Local<v8::Value> value) {
    auto maybeInt32 = Nan::To<v8::Int32>(value);
    if (maybeInt32.IsEmpty()) {
      return Validation<uint8_t>::Invalid("Expected an 8-bit unsigned integer");
    }
    auto int32 = (*maybeInt32.ToLocalChecked())->Value();
    if (int32 < 0 || int32 > 255) {
      return Validation<uint8_t>::Invalid("Expected an 8-bit unsigned integer");
    }
    uint8_t uint8 = static_cast<uint8_t>(int32);
    return Validation<uint8_t>(uint8);
  }
};

template <>
struct Converter<v8::Local<v8::Value>, uint16_t> {
  static Validation<uint16_t> Convert(const v8::Local<v8::Value> value) {
    auto maybeInt32 = Nan::To<v8::Int32>(value);
    if (maybeInt32.IsEmpty()) {
      return Validation<uint16_t>::Invalid("Expected a 16-bit unsigned integer");
    }
    auto int32 = (*maybeInt32.ToLocalChecked())->Value();
    if (int32 < 0 || int32 > 65535) {
      return Validation<uint16_t>::Invalid("Expected a 16-bit unsigned integer");
    }
    uint16_t uint16 = static_cast<uint16_t>(int32);
    return Validation<uint16_t>(uint16);
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
    auto maybeString = value->ToString();
    if (maybeString.IsEmpty()) {
      return Validation<std::string>::Invalid("Expected a string");
    }
    auto string = std::string(*v8::String::Utf8Value(maybeString));
    return Validation<std::string>::Valid(string);
  }
};

template <>
struct Converter<std::string, v8::Local<v8::Value>> {
  static Validation<v8::Local<v8::Value>> Convert(const std::string value) {
    Nan::EscapableHandleScope scope;
    return Validation<v8::Local<v8::Value>>::Valid(scope.Escape(Nan::New(value).ToLocalChecked()));
  }
};

template <typename T>
struct Converter<v8::Local<v8::Value>, std::vector<T>> {
  static Validation<std::vector<T>> Convert(const v8::Local<v8::Value> value) {
    if (!value->IsArray()) {
      return Validation<std::vector<T>>::Invalid("Expected an array");
    }
    auto array = v8::Local<v8::Array>::Cast(value);
    auto validated = std::vector<Validation<T>>();
    for (uint32_t i = 0; i < array->Length(); i++) {
      validated.push_back(From<T>(array->Get(i)));
    }
    return Validation<T>::Sequence(validated);
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
    return Validation<v8::Local<v8::Value>>::Valid(scope.Escape(array));
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

template <>
struct Converter<v8::Local<v8::Value>, v8::Local<v8::External>> {
  static Validation<v8::Local<v8::External>> Convert(const v8::Local<v8::Value> value) {
    Nan::EscapableHandleScope scope;
    return !value.IsEmpty() && value->IsExternal()
        ? Validation<v8::Local<v8::External>>::Valid(scope.Escape(v8::Local<v8::External>::Cast(value)))
        : Validation<v8::Local<v8::External>>::Invalid("Expected an external");
  }
};

}  // namespace node_webrtc

#endif  // SRC_CONVERTERS_V8_H_
