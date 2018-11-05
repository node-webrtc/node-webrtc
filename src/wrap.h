/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_WRAP_H_
#define SRC_WRAP_H_

#include <nan.h>

#include "src/bidimap.h"

namespace node_webrtc {

template<typename T, typename U, typename ...V>
class Wrap {
 public:
  Wrap() = delete;

  explicit Wrap(T(*Create)(V..., U)): _Create(Create) {}

  Wrap(Wrap const&) = delete;

  Wrap& operator=(Wrap const&) = delete;

  T GetOrCreate(V... args, U key) {
    return _map.computeIfAbsent(key, [this, key, args...]() {
      return _Create(args..., key);
    });
  }

  void Release(T value) {
    _map.reverseRemove(value);
  }

 private:
  T(*_Create)(V..., U);
  BidiMap<U, T> _map;
};

}  // namespace webrtc

#endif  // SRC_WRAP_H_
