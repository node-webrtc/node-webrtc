/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_I420HELPERS_H_
#define SRC_I420HELPERS_H_

#include <nan.h>
#include <v8.h>  // IWYU pragma: keep

namespace node_webrtc {

class I420Helpers {
 public:
  static void Init(v8::Handle<v8::Object> exports);

 private:
  static NAN_METHOD(ARGB32ToI420);
  static NAN_METHOD(I420ToARGB32);
};

}  // namespace node_webrtc

#endif  // SRC_I420HELPERS_H_
