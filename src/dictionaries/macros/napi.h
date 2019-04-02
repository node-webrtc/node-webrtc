#pragma once

#include <utility>

#include <node-addon-api/napi.h>

#include "src/converters.h"
#include "src/converters/macros.h"
#include "src/converters/napi.h"
#include "src/functional/validation.h"

namespace node_webrtc {

#define NODE_WEBRTC_CREATE_OBJECT_OR_RETURN(E, O) \
  auto NODE_WEBRTC_UNIQUE_NAME(O) = napi::CreateObject(E); \
  if (NODE_WEBRTC_UNIQUE_NAME(O).IsInvalid()) { \
    return Validation<Napi::Value>::Invalid(NODE_WEBRTC_UNIQUE_NAME(O).ToErrors()); \
  } \
  auto O = NODE_WEBRTC_UNIQUE_NAME(O).UnsafeFromValid();

#define NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(E, O, K, V) \
  { \
    auto maybeErrors = napi::ConvertAndSet(E, O, K, V); \
    if (maybeErrors.IsJust()) { \
      return Validation<Napi::Value>::Invalid(maybeErrors.UnsafeFromJust()); \
    } \
  }

namespace napi {

static Validation<Napi::Object> CreateObject(const Napi::Env env) {
  Napi::EscapableHandleScope scope(env);
  auto maybeObject = Napi::Object::New(env);
  return maybeObject.Env().IsExceptionPending()
      ? Validation<Napi::Object>::Invalid(maybeObject.Env().GetAndClearPendingException().Message())
      : Pure(maybeObject);
}

template <typename T>
static Maybe<Errors> ConvertAndSet(const Napi::Env env, Napi::Object object, const std::string& key, T value) {
  auto maybeValue = From<Napi::Value>(std::make_pair(env, value));
  if (maybeValue.IsInvalid()) {
    return MakeJust(maybeValue.ToErrors());
  }
  object.Set(key, maybeValue.UnsafeFromValid());
  if (object.Env().IsExceptionPending()) {
    std::vector<Error> errors = { object.Env().GetAndClearPendingException().Message() };
    return MakeJust(errors);
  }
  return MakeNothing<Errors>();
}

}  // namespace napi

}  // namespace node_webrtc
