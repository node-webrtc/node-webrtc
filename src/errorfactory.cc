/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/errorfactory.h"

#include <string>

#include "src/converters.h"
#include "src/converters/object.h"
#include "src/converters/v8.h"  // IWYU pragma: keep

namespace node_webrtc {

template <typename T> class Maybe;
template <typename T> class Validation;

}  // namespace node_webrtc

using node_webrtc::ErrorFactory;
using node_webrtc::From;
using node_webrtc::GetRequired;
using node_webrtc::Maybe;
using node_webrtc::Validation;
using v8::Function;
using v8::Local;
using v8::Object;
using v8::Value;

Nan::Persistent<Function> ErrorFactory::DOMException;

void ErrorFactory::Init(Local<Object> module) {
  Nan::TryCatch tc;
  auto maybeRequire = GetRequired<Local<Function>>(module, "require");
  if (tc.HasCaught() || maybeRequire.IsInvalid()) {
    return;
  }
  auto require = maybeRequire.UnsafeFromValid();
  Local<Value> argv = Nan::New("domexception").ToLocalChecked();
  auto result = Nan::Call(require, module, 1, &argv);
  if (tc.HasCaught() || result.IsEmpty()) {
    return;
  }
  auto maybeDOMException = From<Local<Function>>(result.ToLocalChecked());
  if (tc.HasCaught() || maybeDOMException.IsInvalid()) {
    return;
  }
  DOMException.Reset(maybeDOMException.UnsafeFromValid());
}

Local<Value> ErrorFactory::CreateError(const std::string message) {
  Nan::EscapableHandleScope scope;
  return scope.Escape(Nan::Error(Nan::New(message).ToLocalChecked()));
}

Local<Value> ErrorFactory::CreateInvalidAccessError(const std::string message) {
  Nan::EscapableHandleScope scope;
  return scope.Escape(CreateDOMException(message, kInvalidAccessError));
}

Local<Value> ErrorFactory::CreateInvalidModificationError(const std::string message) {
  Nan::EscapableHandleScope scope;
  return scope.Escape(CreateDOMException(message, kInvalidModificationError));
}

Local<Value> ErrorFactory::CreateInvalidStateError(const std::string message) {
  Nan::EscapableHandleScope scope;
  return scope.Escape(CreateDOMException(message, kInvalidStateError));
}

Local<Value> ErrorFactory::CreateNetworkError(const std::string message) {
  Nan::EscapableHandleScope scope;
  return scope.Escape(CreateDOMException(message, kNetworkError));
}

Local<Value> ErrorFactory::CreateOperationError(const std::string message) {
  Nan::EscapableHandleScope scope;
  return scope.Escape(CreateDOMException(message, kOperationError));
}

Local<Value> ErrorFactory::CreateRangeError(const std::string message) {
  Nan::EscapableHandleScope scope;
  return scope.Escape(Nan::RangeError(Nan::New(message).ToLocalChecked()));
}

Local<Value> ErrorFactory::CreateSyntaxError(const std::string message) {
  Nan::EscapableHandleScope scope;
  return scope.Escape(Nan::SyntaxError(Nan::New(message).ToLocalChecked()));
}

const char* ErrorFactory::DOMExceptionNameToString(DOMExceptionName name) {
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

Local<Value> ErrorFactory::CreateDOMException(const std::string message, const DOMExceptionName name) {
  Nan::EscapableHandleScope scope;
  auto prefix = DOMExceptionNameToString(name);
  if (!DOMException.IsEmpty()) {
    auto constructor = Nan::New(DOMException);
    Local<Value> cargv[2];
    cargv[0] = Nan::New(message).ToLocalChecked();
    cargv[1] = Nan::New(prefix).ToLocalChecked();
    auto result = Nan::NewInstance(constructor, 2, cargv);
    if (!result.IsEmpty()) {
      return scope.Escape(result.ToLocalChecked());
    }
  }
  return scope.Escape(Nan::Error(Nan::New(std::string(prefix) + ": " + message).ToLocalChecked()));
}
