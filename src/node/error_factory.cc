/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "error_factory.h"

#include <string>

#include "src/converters.h"
#include "src/converters/napi.h"  // IWYU pragma: keep
#include "src/functional/validation.h"

Napi::FunctionReference& node_webrtc::ErrorFactory::_DOMException() {
  static Napi::FunctionReference func;
  return func;
}

void node_webrtc::ErrorFactory::Init(Napi::Env env, Napi::Object exports) {
  exports.Set("setDOMException", Napi::Function::New(env, SetDOMException));
}

Napi::Value node_webrtc::ErrorFactory::CreateError(const Napi::Env env, const std::string message) {
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(Napi::Error::New(env, message).Value());
}

Napi::Value node_webrtc::ErrorFactory::CreateInvalidAccessError(const Napi::Env env, const std::string message) {
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(CreateDOMException(env, message, DOMExceptionName::kInvalidAccessError));
}

Napi::Value node_webrtc::ErrorFactory::CreateInvalidModificationError(const Napi::Env env, const std::string message) {
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(CreateDOMException(env, message, DOMExceptionName::kInvalidModificationError));
}

Napi::Value node_webrtc::ErrorFactory::CreateInvalidStateError(const Napi::Env env, const std::string message) {
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(CreateDOMException(env, message, DOMExceptionName::kInvalidStateError));
}

Napi::Value node_webrtc::ErrorFactory::CreateNetworkError(const Napi::Env env, const std::string message) {
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(CreateDOMException(env, message, DOMExceptionName::kNetworkError));
}

Napi::Value node_webrtc::ErrorFactory::CreateOperationError(const Napi::Env env, const std::string message) {
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(CreateDOMException(env, message, DOMExceptionName::kOperationError));
}

Napi::Value node_webrtc::ErrorFactory::CreateRangeError(const Napi::Env env, const std::string message) {
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(Napi::RangeError::New(env, message).Value());
}

// FIXME(mroberts): Actually implement this.
Napi::Value node_webrtc::ErrorFactory::CreateSyntaxError(const Napi::Env env, const std::string message) {
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(Napi::Error::New(env, message).Value());
}

const char* node_webrtc::ErrorFactory::DOMExceptionNameToString(DOMExceptionName name) {
  switch (name) {
    case kInvalidAccessError:
      return "InvalidAccessError";
    case kInvalidModificationError:
      return "InvalidModificationError";
    case kInvalidStateError:
      return "InvalidStateError";
    case kNetworkError:
      return "NetworkError";
    case kOperationError:
      return "OperationError";
  }
}

Napi::Value node_webrtc::ErrorFactory::CreateDOMException(Napi::Env env, const std::string message, const DOMExceptionName name) {
  Napi::EscapableHandleScope scope(env);
  auto prefix = DOMExceptionNameToString(name);
  if (!_DOMException().IsEmpty()) {
    return scope.Escape(_DOMException().New({
      Napi::String::New(env, message),
      Napi::String::New(env, prefix)
    }));
  }
  return scope.Escape(Napi::Error::New(env, std::string(prefix) + ": " + message).Value());
}

Napi::Value node_webrtc::ErrorFactory::SetDOMException(const Napi::CallbackInfo& info) {
  auto maybeDOMException = node_webrtc::From<Napi::Function>(info[0]);
  if (maybeDOMException.IsInvalid()) {
    Napi::TypeError::New(info.Env(), maybeDOMException.ToErrors()[0]).ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }
  _DOMException() = Napi::Persistent(maybeDOMException.UnsafeFromValid());
  _DOMException().SuppressDestruct();
  return info.Env().Undefined();
}
