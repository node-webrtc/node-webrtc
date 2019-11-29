/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifdef DEBUG

#pragma once

#include <node-addon-api/napi.h>

namespace node_webrtc {

class Test {
 public:
  static void Init(Napi::Env, Napi::Object);

  static Napi::Env* env;

 private:
  static Napi::Value TestImpl(const Napi::CallbackInfo&);
};

}  // namespace node_webrtc

#endif  // DEBUG
