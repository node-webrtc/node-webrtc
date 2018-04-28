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

class ObjectWrap: public Nan::ObjectWrap {
 public:
  void AddRef() {
    Ref();
  }

  void RemoveRef() {
    Unref();
  }
};

}  // namespace node_webrtc

#endif  // SRC_OBJECTWRAP_H_
