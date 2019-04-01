/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/converters/v8.h"

#include <string>

namespace node_webrtc {

FROM_JS_IMPL(bool, value) {
  auto maybeBoolean = Nan::To<v8::Boolean>(value);
  if (maybeBoolean.IsEmpty()) {
    return Validation<bool>::Invalid("Expected a bool");
  }
  auto boolean = (*maybeBoolean.ToLocalChecked())->Value();
  return Pure(boolean);
}

TO_JS_IMPL(bool, value) {
  Nan::EscapableHandleScope scope;
  return value
      ? Pure(scope.Escape(Nan::True()).As<v8::Value>())
      : Pure(scope.Escape(Nan::False()).As<v8::Value>());
}

FROM_JS_IMPL(double, value) {
  auto maybeNumber = Nan::To<v8::Number>(value);
  if (maybeNumber.IsEmpty()) {
    return Validation<double>::Invalid("Expected a double");
  }
  auto number = (*maybeNumber.ToLocalChecked())->Value();
  return Pure(number);
}

TO_JS_IMPL(double, value) {
  Nan::EscapableHandleScope scope;
  return Pure(scope.Escape(Nan::New(value)).As<v8::Value>());
}

FROM_JS_IMPL(int32_t, value) {
  auto maybeInt32 = Nan::To<v8::Int32>(value);
  if (maybeInt32.IsEmpty()) {
    return Validation<int32_t>::Invalid("Expected a 32-bit integer");
  }
  auto int32 = (*maybeInt32.ToLocalChecked())->Value();
  return Pure(int32);
}

TO_JS_IMPL(int32_t, value) {
  Nan::EscapableHandleScope scope;
  return Pure(scope.Escape(Nan::New(value)).As<v8::Value>());
}

FROM_JS_IMPL(int64_t, value) {
  auto maybeInteger = Nan::To<v8::Integer>(value);
  if (maybeInteger.IsEmpty()) {
    return Validation<int64_t>::Invalid("Expected a 64-bit integer");
  }
  auto integer = (*maybeInteger.ToLocalChecked())->Value();
  return Pure(integer);
}

TO_JS_IMPL(int64_t, value) {
  Nan::EscapableHandleScope scope;
  // NOTE(mroberts): Is this correct?
  return Pure(scope.Escape(Nan::New(static_cast<double>(value))).As<v8::Value>());
}

FROM_JS_IMPL(uint8_t, value) {
  auto maybeInt32 = Nan::To<v8::Int32>(value);
  if (maybeInt32.IsEmpty()) {
    return Validation<uint8_t>::Invalid("Expected an 8-bit unsigned integer");
  }
  auto int32 = (*maybeInt32.ToLocalChecked())->Value();
  if (int32 < 0 || int32 > UINT8_MAX) {
    return Validation<uint8_t>::Invalid("Expected an 8-bit unsigned integer");
  }
  auto uint8 = static_cast<uint8_t>(int32);
  return Validation<uint8_t>(uint8);
}

TO_JS_IMPL(uint8_t, value) {
  Nan::EscapableHandleScope scope;
  return Pure(scope.Escape(Nan::New(value)).As<v8::Value>());
}

FROM_JS_IMPL(uint16_t, value) {
  auto maybeInt32 = Nan::To<v8::Int32>(value);
  if (maybeInt32.IsEmpty()) {
    return Validation<uint16_t>::Invalid("Expected a 16-bit unsigned integer");
  }
  auto int32 = (*maybeInt32.ToLocalChecked())->Value();
  if (int32 < 0 || int32 > UINT16_MAX) {
    return Validation<uint16_t>::Invalid("Expected a 16-bit unsigned integer");
  }
  auto uint16 = static_cast<uint16_t>(int32);
  return Validation<uint16_t>(uint16);
}

TO_JS_IMPL(uint16_t, value) {
  Nan::EscapableHandleScope scope;
  return Pure(scope.Escape(Nan::New(value)).As<v8::Value>());
}

FROM_JS_IMPL(uint32_t, value) {
  auto maybeUint32 = Nan::To<v8::Uint32>(value);
  if (maybeUint32.IsEmpty()) {
    return Validation<uint32_t>::Invalid("Expected a 32-bit unsigned integer");
  }
  auto uint32 = (*maybeUint32.ToLocalChecked())->Value();
  return Pure(uint32);
}

TO_JS_IMPL(uint32_t, value) {
  Nan::EscapableHandleScope scope;
  return Pure(scope.Escape(Nan::New(value)).As<v8::Value>());
}

TO_JS_IMPL(uint64_t, value) {
  Nan::EscapableHandleScope scope;
  // NOTE(mroberts): Is this correct?
  return Pure(scope.Escape(Nan::New(static_cast<double>(value))).As<v8::Value>());
}

FROM_JS_IMPL(std::string, value) {
  auto maybeString = value->ToString();
  if (maybeString.IsEmpty()) {
    return Validation<std::string>::Invalid("Expected a string");
  }
  auto string = std::string(*v8::String::Utf8Value(maybeString));
  return Pure(string);
}

TO_JS_IMPL(std::string, value) {
  Nan::EscapableHandleScope scope;
  return Pure(scope.Escape(Nan::New(value).ToLocalChecked()).As<v8::Value>());
}

FROM_JS_IMPL(v8::Local<v8::External>, value) {
  Nan::EscapableHandleScope scope;
  return !value.IsEmpty() && value->IsExternal()
      ? Pure(scope.Escape(value).As<v8::External>())
      : Validation<v8::Local<v8::External>>::Invalid("Expected an external");
}

FROM_JS_IMPL(v8::Local<v8::Function>, value) {
  if (!value->IsFunction()) {
    return Validation<v8::Local<v8::Function>>::Invalid("Expected a function");
  }
  auto function = v8::Local<v8::Function>::Cast(value);
  return Pure(function);
}

FROM_JS_IMPL(v8::Local<v8::Object>, value) {
  Nan::EscapableHandleScope scope;
  if (!value->IsObject() || value->IsNull() || value->IsUndefined()) {
    return Validation<v8::Local<v8::Object>>::Invalid("Expected an object");
  }
  auto object = value.As<v8::Object>();
  return Pure(scope.Escape(object));
}

TO_JS_IMPL(v8::Local<v8::Primitive>, value) {
  Nan::EscapableHandleScope scope;
  return Pure(scope.Escape(value.As<v8::Value>()));
}

TO_JS_IMPL(std::vector<bool>, values) {
  Nan::EscapableHandleScope scope;
  auto array = Nan::New<v8::Array>();
  uint32_t i = 0;
  for (bool value : values) {
    auto maybeValue = From<v8::Local<v8::Value>>(value);
    if (maybeValue.IsInvalid()) {
      return Validation<v8::Local<v8::Value>>::Invalid(maybeValue.ToErrors());
    }
    array->Set(i++, maybeValue.UnsafeFromValid());
  }
  return Pure(scope.Escape(array).As<v8::Value>());
}

FROM_JS_IMPL(v8::ArrayBuffer::Contents, value) {
  return value->IsArrayBufferView()
      ? Pure(value.As<v8::ArrayBufferView>()->Buffer()->GetContents())
      : Validation<v8::ArrayBuffer::Contents>::Invalid("Expected an ArrayBuffer");
}

}  // namespace node_webrtc
