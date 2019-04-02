#include "src/converters/napi.h"

#include "src/functional/validation.h"

namespace node_webrtc {

FROM_NAPI_IMPL(bool, value) {
  auto maybeBoolean = value.ToBoolean();
  return maybeBoolean.Env().IsExceptionPending()
      ? Validation<bool>::Invalid(maybeBoolean.Env().GetAndClearPendingException().Message())
      : Pure(maybeBoolean.Value());
}

TO_NAPI_IMPL(bool, pair) {
  Napi::EscapableHandleScope scope(pair.first);
  return Pure(scope.Escape(Napi::Boolean::New(pair.first, pair.second).As<Napi::Value>()));
}

FROM_NAPI_IMPL(double, value) {
  auto maybeNumber = value.ToNumber();
  return maybeNumber.Env().IsExceptionPending()
      ? Validation<double>::Invalid(maybeNumber.Env().GetAndClearPendingException().Message())
      : Pure(maybeNumber.DoubleValue());
}

TO_NAPI_IMPL(double, pair) {
  Napi::EscapableHandleScope scope(pair.first);
  return Pure(scope.Escape(Napi::Number::New(pair.first, pair.second).As<Napi::Value>()));
}

FROM_NAPI_IMPL(uint8_t, value) {
  auto maybeNumber = value.ToNumber();
  if (maybeNumber.Env().IsExceptionPending()) {
    return Validation<uint8_t>::Invalid(maybeNumber.Env().GetAndClearPendingException().Message());
  }
  auto doubleValue = maybeNumber.DoubleValue();
  if (doubleValue < 0 || doubleValue > UINT8_MAX) {
    return Validation<uint8_t>::Invalid("Expected a 8-bit unsigned integer");
  }
  return Pure(static_cast<uint8_t>(maybeNumber.Uint32Value()));
}

TO_NAPI_IMPL(uint8_t, pair) {
  Napi::EscapableHandleScope scope(pair.first);
  return Pure(scope.Escape(Napi::Number::New(pair.first, pair.second).As<Napi::Value>()));
}

FROM_NAPI_IMPL(uint16_t, value) {
  auto maybeNumber = value.ToNumber();
  if (maybeNumber.Env().IsExceptionPending()) {
    return Validation<uint16_t>::Invalid(maybeNumber.Env().GetAndClearPendingException().Message());
  }
  auto doubleValue = maybeNumber.DoubleValue();
  if (doubleValue < 0 || doubleValue > UINT16_MAX) {
    return Validation<uint16_t>::Invalid("Expected a 16-bit unsigned integer");
  }
  return Pure(static_cast<uint16_t>(maybeNumber.Uint32Value()));
}

TO_NAPI_IMPL(uint16_t, pair) {
  Napi::EscapableHandleScope scope(pair.first);
  return Pure(scope.Escape(Napi::Number::New(pair.first, pair.second).As<Napi::Value>()));
}

FROM_NAPI_IMPL(uint32_t, value) {
  auto maybeNumber = value.ToNumber();
  if (maybeNumber.Env().IsExceptionPending()) {
    return Validation<uint32_t>::Invalid(maybeNumber.Env().GetAndClearPendingException().Message());
  }
  auto doubleValue = maybeNumber.DoubleValue();
  if (doubleValue < 0 || doubleValue > UINT32_MAX) {
    return Validation<uint32_t>::Invalid("Expected a 32-bit unsigned integer");
  }
  return Pure(maybeNumber.Uint32Value());
}

TO_NAPI_IMPL(uint32_t, pair) {
  Napi::EscapableHandleScope scope(pair.first);
  return Pure(scope.Escape(Napi::Number::New(pair.first, pair.second).As<Napi::Value>()));
}

FROM_NAPI_IMPL(uint64_t, value) {
  auto maybeNumber = value.ToNumber();
  if (maybeNumber.Env().IsExceptionPending()) {
    return Validation<uint64_t>::Invalid(maybeNumber.Env().GetAndClearPendingException().Message());
  }
  auto doubleValue = maybeNumber.DoubleValue();
  if (doubleValue < 0 || doubleValue > UINT64_MAX) {
    return Validation<uint64_t>::Invalid("Expected a 64-bit unsigned integer");
  }
  return Pure(static_cast<uint64_t>(maybeNumber.DoubleValue()));
}

TO_NAPI_IMPL(uint64_t, pair) {
  Napi::EscapableHandleScope scope(pair.first);
  return Pure(scope.Escape(Napi::Number::New(pair.first, pair.second).As<Napi::Value>()));
}

FROM_NAPI_IMPL(int8_t, value) {
  auto maybeNumber = value.ToNumber();
  if (maybeNumber.Env().IsExceptionPending()) {
    return Validation<int8_t>::Invalid(maybeNumber.Env().GetAndClearPendingException().Message());
  }
  auto doubleValue = maybeNumber.DoubleValue();
  if (doubleValue < INT8_MIN || doubleValue > INT8_MAX) {
    return Validation<int8_t>::Invalid("Expected a 8-bit integer");
  }
  return Pure(static_cast<int8_t>(maybeNumber.Int32Value()));
}

FROM_NAPI_IMPL(int16_t, value) {
  auto maybeNumber = value.ToNumber();
  if (maybeNumber.Env().IsExceptionPending()) {
    return Validation<int16_t>::Invalid(maybeNumber.Env().GetAndClearPendingException().Message());
  }
  auto doubleValue = maybeNumber.DoubleValue();
  if (doubleValue < INT16_MIN || doubleValue > INT16_MAX) {
    return Validation<int16_t>::Invalid("Expected a 16-bit integer");
  }
  return Pure(static_cast<int16_t>(maybeNumber.Int32Value()));
}

TO_NAPI_IMPL(int16_t, pair) {
  Napi::EscapableHandleScope scope(pair.first);
  return Pure(scope.Escape(Napi::Number::New(pair.first, pair.second).As<Napi::Value>()));
}

FROM_NAPI_IMPL(int32_t, value) {
  auto maybeNumber = value.ToNumber();
  if (maybeNumber.Env().IsExceptionPending()) {
    return Validation<int32_t>::Invalid(maybeNumber.Env().GetAndClearPendingException().Message());
  }
  auto doubleValue = maybeNumber.DoubleValue();
  if (doubleValue < INT32_MIN || doubleValue > INT32_MAX) {
    return Validation<int32_t>::Invalid("Expected a 32-bit integer");
  }
  return Pure(maybeNumber.Int32Value());
}

TO_NAPI_IMPL(int32_t, pair) {
  Napi::EscapableHandleScope scope(pair.first);
  return Pure(scope.Escape(Napi::Number::New(pair.first, pair.second).As<Napi::Value>()));
}

FROM_NAPI_IMPL(int64_t, value) {
  auto maybeNumber = value.ToNumber();
  if (maybeNumber.Env().IsExceptionPending()) {
    return Validation<int64_t>::Invalid(maybeNumber.Env().GetAndClearPendingException().Message());
  }
  auto doubleValue = maybeNumber.DoubleValue();
  if (doubleValue < INT64_MIN || doubleValue > INT64_MAX) {
    return Validation<int64_t>::Invalid("Expected a 64-bit integer");
  }
  return Pure(maybeNumber.Int64Value());
}

TO_NAPI_IMPL(int64_t, pair) {
  Napi::EscapableHandleScope scope(pair.first);
  return Pure(scope.Escape(Napi::Number::New(pair.first, pair.second).As<Napi::Value>()));
}

FROM_NAPI_IMPL(std::string, value) {
  auto maybeString = value.ToString();
  return maybeString.Env().IsExceptionPending()
      ? Validation<std::string>::Invalid("Expected a string")
      : Pure(maybeString.Utf8Value());
}

TO_NAPI_IMPL(std::string, pair) {
  Napi::EscapableHandleScope scope(pair.first);
  auto maybeValue = Napi::String::New(pair.first, pair.second);
  return maybeValue.Env().IsExceptionPending()
      ? Validation<Napi::Value>::Invalid(maybeValue.Env().GetAndClearPendingException().Message())
      : Pure(scope.Escape(maybeValue.As<Napi::Value>()));
}

FROM_NAPI_IMPL(Napi::Function, value) {
  return value.IsFunction()
      ? Pure(value.As<Napi::Function>())
      : Validation<Napi::Function>::Invalid("Expected a function");
}

FROM_NAPI_IMPL(Napi::Object, value) {
  return value.IsObject()
      ? Pure(value.As<Napi::Object>())
      : Validation<Napi::Object>::Invalid("Expected an object");
}

TO_NAPI_IMPL(std::vector<bool>, pair) {
  Napi::EscapableHandleScope scope(pair.first);
  auto maybeArray = Napi::Array::New(pair.first, pair.second.size());
  if (maybeArray.Env().IsExceptionPending()) {
    return Validation<Napi::Value>::Invalid(maybeArray.Env().GetAndClearPendingException().Message());
  }
  uint32_t i = 0;
  for (bool value : pair.second) {
    maybeArray.Set(i++, Napi::Boolean::New(pair.first, value));
    if (maybeArray.Env().IsExceptionPending()) {
      return Validation<Napi::Value>::Invalid(maybeArray.Env().GetAndClearPendingException().Message());
    }
  }
  return Pure(scope.Escape(maybeArray.As<Napi::Value>()));
}

FROM_NAPI_IMPL(Napi::ArrayBuffer, value) {
  return value.IsArrayBuffer()
      ? Pure(value.As<Napi::ArrayBuffer>())
      : Validation<Napi::ArrayBuffer>::Invalid("Expected an ArrayBuffer");
}

}  // namespace node_webrtc
