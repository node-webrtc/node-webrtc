/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
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
#include "src/functional/validation.h"

// IWYU pragma: no_include <nan_implementation_12_inl.h>

Nan::Persistent<v8::Function> node_webrtc::ErrorFactory::DOMException;

void node_webrtc::ErrorFactory::Init(v8::Local<v8::Object> module) {
  Nan::TryCatch tc;
  auto maybeRequire = node_webrtc::GetRequired<v8::Local<v8::Function>>(module, "require");
  if (tc.HasCaught() || maybeRequire.IsInvalid()) {
    return;
  }
  auto require = maybeRequire.UnsafeFromValid();
  v8::Local<v8::Value> argv = Nan::New("domexception").ToLocalChecked();
  auto result = Nan::Call(require, module, 1, &argv);
  if (tc.HasCaught() || result.IsEmpty()) {
    return;
  }
  auto maybeDOMException = node_webrtc::From<v8::Local<v8::Function>>(result.ToLocalChecked());
  if (tc.HasCaught() || maybeDOMException.IsInvalid()) {
    return;
  }
  DOMException.Reset(maybeDOMException.UnsafeFromValid());
}

v8::Local<v8::Value> node_webrtc::ErrorFactory::CreateError(const std::string message) {
  Nan::EscapableHandleScope scope;
  return scope.Escape(Nan::Error(Nan::New(message).ToLocalChecked()));
}

v8::Local<v8::Value> node_webrtc::ErrorFactory::CreateInvalidAccessError(const std::string message) {
  Nan::EscapableHandleScope scope;
  return scope.Escape(CreateDOMException(message, kInvalidAccessError));
}

v8::Local<v8::Value> node_webrtc::ErrorFactory::CreateInvalidModificationError(const std::string message) {
  Nan::EscapableHandleScope scope;
  return scope.Escape(CreateDOMException(message, kInvalidModificationError));
}

v8::Local<v8::Value> node_webrtc::ErrorFactory::CreateInvalidStateError(const std::string message) {
  Nan::EscapableHandleScope scope;
  return scope.Escape(CreateDOMException(message, kInvalidStateError));
}

v8::Local<v8::Value> node_webrtc::ErrorFactory::CreateNetworkError(const std::string message) {
  Nan::EscapableHandleScope scope;
  return scope.Escape(CreateDOMException(message, kNetworkError));
}

v8::Local<v8::Value> node_webrtc::ErrorFactory::CreateOperationError(const std::string message) {
  Nan::EscapableHandleScope scope;
  return scope.Escape(CreateDOMException(message, kOperationError));
}

v8::Local<v8::Value> node_webrtc::ErrorFactory::CreateRangeError(const std::string message) {
  Nan::EscapableHandleScope scope;
  return scope.Escape(Nan::RangeError(Nan::New(message).ToLocalChecked()));
}

v8::Local<v8::Value> node_webrtc::ErrorFactory::CreateSyntaxError(const std::string message) {
  Nan::EscapableHandleScope scope;
  return scope.Escape(Nan::SyntaxError(Nan::New(message).ToLocalChecked()));
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

v8::Local<v8::Value> node_webrtc::ErrorFactory::CreateDOMException(const std::string message, const DOMExceptionName name) {
  Nan::EscapableHandleScope scope;
  auto prefix = DOMExceptionNameToString(name);
  if (!DOMException.IsEmpty()) {
    auto constructor = Nan::New(DOMException);
    v8::Local<v8::Value> cargv[2];
    cargv[0] = Nan::New(message).ToLocalChecked();
    cargv[1] = Nan::New(prefix).ToLocalChecked();
    auto result = Nan::NewInstance(constructor, 2, cargv);
    if (!result.IsEmpty()) {
      return scope.Escape(result.ToLocalChecked());
    }
  }
  return scope.Escape(Nan::Error(Nan::New(std::string(prefix) + ": " + message).ToLocalChecked()));
}
