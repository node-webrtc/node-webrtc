/* Copyright (c) 2017 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef TEST_H_
#define TEST_H_
#ifdef DEBUG

#include "v8.h"

namespace node_webrtc {

class Test {
 public:
  static void Init(v8::Handle<v8::Object> exports);
};

}  // namespace node_webrtc

#endif  // DEBUG
#endif  // TEST_H_
