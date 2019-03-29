/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <nan.h>
#include <uv.h>

#include "event_loop.h"

namespace node_webrtc {

/**
 * A PromiseFulfillingEventLoop is an EventLoop that can also fulfill Promises.
 * @tparam T the Event target type
 */
template <typename T>
class PromiseFulfillingEventLoop: public EventLoop<T> {
 protected:
  explicit PromiseFulfillingEventLoop(T& target): EventLoop<T>(target) {}

  ~PromiseFulfillingEventLoop() override = default;

  void Run() override {
    Nan::HandleScope scope;
    EventLoop<T>::Run();
    if (!this->should_stop()) {
      Nan::GetCurrentContext()->GetIsolate()->RunMicrotasks();
    }
  }
};

}  // namespace node_webrtc
