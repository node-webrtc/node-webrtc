/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_OBJECTWRAP_H_
#define SRC_OBJECTWRAP_H_

#include "nan.h"

namespace node_webrtc {

class ObjectWrap: private Nan::ObjectWrap {
 public:
  inline v8::Local<v8::Object> ToObject() {
    Nan::EscapableHandleScope scope;
    return scope.Escape(handle());
  }

  template <typename T>
  inline static T* Unwrap(v8::Local<v8::Object> object) {
    auto unwrapped = Nan::ObjectWrap::Unwrap<Nan::ObjectWrap>(object);
    return reinterpret_cast<T*>(unwrapped);
  }

  inline void Wrap(v8::Local<v8::Object> object) {
    Nan::ObjectWrap::Wrap(object);
  }

 protected:
  inline void Ref() {
    Nan::ObjectWrap::Ref();
  }

  inline void Unref() {
    Nan::ObjectWrap::Unref();
  }
};

}  // namespace node_webrtc

#endif  // SRC_OBJECTWRAP_H_
