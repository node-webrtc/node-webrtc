#pragma once

#include <cstdint>
#include <iosfwd>
#include <string>
#include <string.h>
#include <vector>
#include <utility>

#include <node-addon-api/napi.h>
#include <v8.h>

#include "src/converters.h"
#include "src/converters/macros.h"
#include "src/functional/maybe.h"
#include "src/functional/validation.h"

#define CONVERT_OR_THROW_AND_RETURN_NAPI(E, I, O, T) \
  auto NODE_WEBRTC_UNIQUE_NAME(validation) = From<detail::argument_type<void(T)>::type>(std::make_pair(E, I)); \
  if (NODE_WEBRTC_UNIQUE_NAME(validation).IsInvalid()) { \
    auto error = NODE_WEBRTC_UNIQUE_NAME(validation).ToErrors()[0]; \
    Napi::TypeError::New(E, error).ThrowAsJavaScriptException(); \
    return E.Undefined(); \
  } \
  auto O = NODE_WEBRTC_UNIQUE_NAME(validation).UnsafeFromValid();

#define CONVERT_OR_REJECT_AND_RETURN_NAPI(D, I, O, T) \
  auto NODE_WEBRTC_UNIQUE_NAME(validation) = From<detail::argument_type<void(T)>::type>(std::make_pair(D.Env(), I)); \
  if (NODE_WEBRTC_UNIQUE_NAME(validation).IsInvalid()) { \
    auto error = NODE_WEBRTC_UNIQUE_NAME(validation).ToErrors()[0]; \
    D.Reject(Napi::TypeError::New(D.Env(), error)); \
    return D.Env().Undefined(); \
  } \
  auto O = NODE_WEBRTC_UNIQUE_NAME(validation).UnsafeFromValid();

namespace node_webrtc {

namespace napi {

static inline Napi::Value UnsafeFromV8(const Napi::Env env, v8::Local<v8::Value> value) {
  return {env, reinterpret_cast<napi_value>(*value)};
}

static inline v8::Local<v8::Value> UnsafeToV8(napi_value value) {
  v8::Local<v8::Value> local;
  memcpy(&local, &value, sizeof(value));
  return local;
}

}  // namespace napi

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
DECLARE_TO_NAPI(Napi::Value)
DECLARE_TO_NAPI(std::vector<bool>)
DECLARE_FROM_NAPI(Napi::ArrayBuffer)

template <typename T>
struct Converter<Napi::Value, Maybe<T>> {
  static Validation<Maybe<T>> Convert(const Napi::Value value) {
    return value.IsUndefined()
        ? Pure(MakeNothing<T>())
        : From<T>(value).Map(&MakeJust<T>);
  }
};

template <typename T>
struct Converter<std::pair<Napi::Env, Maybe<T>>, Napi::Value> {
  static Validation<Napi::Value> Convert(const std::pair<Napi::Env, Maybe<T>> pair) {
    return pair.second.IsJust()
        ? From<Napi::Value>(std::make_pair(pair.first, pair.second.UnsafeFromJust()))
        : Pure(pair.first.Null());
  }
};

template <>
struct Converter<Napi::Value, Napi::Array> {
  static Validation<Napi::Array> Convert(Napi::Value value) {
    return value.IsArray()
        ? Pure(value.As<Napi::Array>())
        : Validation<Napi::Array>::Invalid(("Expected an array"));
  }
};

template <typename T>
struct Converter<Napi::Array, std::vector<T>> {
  static Validation<std::vector<T>> Convert(const Napi::Array array) {
    auto validated = std::vector<T>();
    validated.reserve(array.Length());
    for (uint32_t i = 0; i < array.Length(); i++) {
      auto maybeValue = array.Get(i);
      if (maybeValue.Env().IsExceptionPending()) {
        return Validation<std::vector<T>>::Invalid(maybeValue.Env().GetAndClearPendingException().Message());
      }
      auto maybeValidated = From<T>(maybeValue);
      if (maybeValidated.IsInvalid()) {
        return Validation<std::vector<T>>::Invalid(maybeValidated.ToErrors());
      }
      validated.push_back(maybeValidated.UnsafeFromValid());
    }
    return Pure(validated);
  }
};

template <typename T>
struct Converter<Napi::Value, std::vector<T>> {
  static Validation<std::vector<T>> Convert(const Napi::Value value) {
    return Converter<Napi::Value, Napi::Array>::Convert(value).FlatMap<std::vector<T>>(Converter<Napi::Array, std::vector<T>>::Convert);
  }
};

template <typename T>
struct Converter<std::pair<Napi::Env, std::vector<T>>, Napi::Value> {
  static Validation<Napi::Value> Convert(std::pair<Napi::Env, std::vector<T>> pair) {
    auto env = pair.first;
    Napi::EscapableHandleScope scope(env);
    auto values = pair.second;
    auto maybeArray = Napi::Array::New(env, values.size());
    if (maybeArray.Env().IsExceptionPending()) {
      return Validation<Napi::Value>::Invalid(maybeArray.Env().GetAndClearPendingException().Message());
    }
    uint32_t i = 0;
    for (const auto& value : values) {
      auto maybeValue = From<Napi::Value>(std::make_pair(env, value));
      if (maybeValue.IsInvalid()) {
        return Validation<Napi::Value>::Invalid(maybeValue.ToErrors());
      }
      maybeArray.Set(i++, maybeValue.UnsafeFromValid());
      if (maybeArray.Env().IsExceptionPending()) {
        return Validation<Napi::Value>::Invalid(maybeArray.Env().GetAndClearPendingException().Message());
      }
    }
    return Pure(scope.Escape(maybeArray));
  }
};

}  // namespace node_webrtc
