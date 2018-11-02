/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_ERRORFACTORY_H_
#define SRC_ERRORFACTORY_H_

#include <iosfwd>

#include <nan.h>  // IWYU pragma: keep
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

  static void Init(v8::Local<v8::Object> module);

  static v8::Local<v8::Value> CreateError(std::string message);
  static v8::Local<v8::Value> CreateInvalidAccessError(std::string message);
  static v8::Local<v8::Value> CreateInvalidModificationError(std::string message);
  static v8::Local<v8::Value> CreateInvalidStateError(std::string message);
  static v8::Local<v8::Value> CreateNetworkError(std::string message);
  static v8::Local<v8::Value> CreateOperationError(std::string message);
  static v8::Local<v8::Value> CreateRangeError(std::string message);
  static v8::Local<v8::Value> CreateSyntaxError(std::string message);

 private:
  static Nan::Persistent<v8::Function> DOMException;

  static const char* DOMExceptionNameToString(DOMExceptionName name);

  static v8::Local<v8::Value> CreateDOMException(std::string message, DOMExceptionName name);
};

}  // namespace node_webrtc

#endif  // SRC_ERRORFACTORY_H_
