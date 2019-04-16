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

void node_webrtc::ErrorFactory::Init(Napi::Env env, Napi::Object exports) {
  exports.Set("setDOMException", Napi::Function::New(env, SetDOMException));
}

Napi::Value node_webrtc::ErrorFactory::napi::CreateError(const Napi::Env env, const std::string message) {
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(Napi::Error::New(env, message).Value());
}

// FIXME(mroberts): Actually implement this.
Napi::Value node_webrtc::ErrorFactory::napi::CreateInvalidAccessError(const Napi::Env env, const std::string message) {
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(Napi::Error::New(env, message).Value());
}

// FIXME(mroberts): Actually implement this.
Napi::Value node_webrtc::ErrorFactory::napi::CreateInvalidModificationError(const Napi::Env env, const std::string message) {
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(Napi::Error::New(env, message).Value());
}

// FIXME(mroberts): Actually implement this.
Napi::Value node_webrtc::ErrorFactory::napi::CreateInvalidStateError(const Napi::Env env, const std::string message) {
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(Napi::Error::New(env, message).Value());
}

// FIXME(mroberts): Actually implement this.
Napi::Value node_webrtc::ErrorFactory::napi::CreateNetworkError(const Napi::Env env, const std::string message) {
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(Napi::Error::New(env, message).Value());
}

// FIXME(mroberts): Actually implement this.
Napi::Value node_webrtc::ErrorFactory::napi::CreateOperationError(const Napi::Env env, const std::string message) {
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(Napi::Error::New(env, message).Value());
}

Napi::Value node_webrtc::ErrorFactory::napi::CreateRangeError(const Napi::Env env, const std::string message) {
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(Napi::RangeError::New(env, message).Value());
}

// FIXME(mroberts): Actually implement this.
Napi::Value node_webrtc::ErrorFactory::napi::CreateSyntaxError(const Napi::Env env, const std::string message) {
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

Napi::Value node_webrtc::ErrorFactory::SetDOMException(const Napi::CallbackInfo& info) {
  auto maybeDOMException = node_webrtc::From<Napi::Function>(info[0]);
  if (maybeDOMException.IsInvalid()) {
    Napi::TypeError::New(info.Env(), maybeDOMException.ToErrors()[0]).ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }
  return info.Env().Undefined();
}
