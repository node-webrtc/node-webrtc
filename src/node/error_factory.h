/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <iosfwd>

#include <nan.h>
#include <node-addon-api/napi.h>
#include <v8.h>

namespace node_webrtc {

class ErrorFactory {
 public:
  enum DOMExceptionName {
    kInvalidAccessError,
    kInvalidModificationError,
    kInvalidStateError,
    kNetworkError,
    kOperationError
  };

  enum ErrorName {
    kError,
    kRangeError,
    kSyntaxError
  };

  static void Init(Napi::Env, Napi::Object);

  static v8::Local<v8::Value> CreateError(std::string message);
  static v8::Local<v8::Value> CreateInvalidAccessError(std::string message);
  static v8::Local<v8::Value> CreateInvalidModificationError(std::string message);
  static v8::Local<v8::Value> CreateInvalidStateError(std::string message);
  static v8::Local<v8::Value> CreateNetworkError(std::string message);
  static v8::Local<v8::Value> CreateOperationError(std::string message);
  static v8::Local<v8::Value> CreateRangeError(std::string message);
  static v8::Local<v8::Value> CreateSyntaxError(std::string message);

  class napi {
   public:
    static Napi::Value CreateError(Napi::Env, std::string);
    static Napi::Value CreateInvalidAccessError(Napi::Env, std::string);
    static Napi::Value CreateInvalidModificationError(Napi::Env, std::string);
    static Napi::Value CreateInvalidStateError(Napi::Env, std::string);
    static Napi::Value CreateNetworkError(Napi::Env, std::string);
    static Napi::Value CreateOperationError(Napi::Env, std::string);
    static Napi::Value CreateRangeError(Napi::Env, std::string);
    static Napi::Value CreateSyntaxError(Napi::Env, std::string);
  };

 private:
  static Napi::Value SetDOMException(const Napi::CallbackInfo&);

  static Nan::Persistent<v8::Function> DOMException;

  static const char* DOMExceptionNameToString(DOMExceptionName name);

  static v8::Local<v8::Value> CreateDOMException(std::string message, DOMExceptionName name);
};

}  // namespace node_webrtc
