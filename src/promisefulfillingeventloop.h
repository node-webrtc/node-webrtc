/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_PROMISEFULFILLINGEVENTLOOP_H_
#define SRC_PROMISEFULFILLINGEVENTLOOP_H_

#include <memory>
#include <queue>

#include "uv.h"

#include "src/eventloop.h"
#include "src/events.h"

namespace node_webrtc {

/**
 * A PromiseFulfillingEventLoop is an EventLoop that can also fulfill Promises.
 * @tparam T the Event target type
 */
template <typename T>
class PromiseFulfillingEventLoop: public EventLoop<T> {
 protected:
  explicit PromiseFulfillingEventLoop(T& target): EventLoop<T>(target) {}

  virtual ~PromiseFulfillingEventLoop() override = default;

  void Run() override {
    Nan::HandleScope scope;
    EventLoop<T>::Run();
    if (!this->should_stop()) {
      Nan::GetCurrentContext()->GetIsolate()->RunMicrotasks();
    }
  }
};

}  // namespace node_webrtc

#endif  // SRC_PROMISEFULFILLINGEVENTLOOP_H_
