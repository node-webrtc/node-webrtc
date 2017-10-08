/* Copyright (c) 2017 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_EVENTLOOP_H_
#define SRC_EVENTLOOP_H_

#include <uv.h>

#include <memory>
#include <queue>

#include "src/eventqueue.h"
#include "src/events.h"

namespace node_webrtc {

/**
 * EventLoop is a thread-safe Event loop. It allows you to dispatch events from
 * one thread and handle them in another (or the same).
 * @tparam T the Event target type
 */
template <typename T>
class EventLoop: EventQueue<T> {
 public:
  /**
   * Dispatch an event to the EventLoop.
   * @param event the event to dispatch
   */
  void Dispatch(std::unique_ptr<Event<T>> event) {
    if (_should_stop) {
      return;
    }
    this->Enqueue(std::move(event));
    uv_async_send(&_async);
  }

 protected:
  explicit EventLoop(T& target): EventQueue<T>(), _loop(uv_default_loop()), _target(target) {
    uv_async_init(_loop, &_async, Callback);
    _async.data = this;
  }

  /**
   * This method will be invoked once the EventLoop stops.
   */
  virtual void DidStop() {
    // Do nothing.
  }

  bool should_stop() const {
    return _should_stop;
  }

  /**
   * Stop the EventLoop.
   */
  void Stop() {
    _should_stop = true;
  }

  virtual void Run() {
    while (auto event = this->Dequeue()) {
      event->Dispatch(this->_target);
    }

    if (this->_should_stop) {
      uv_close(reinterpret_cast<uv_handle_t*>(&this->_async), nullptr);
      this->DidStop();
    }
  }

 private:
  static void Callback(uv_async_t* handle) {
    auto self = reinterpret_cast<EventLoop<T> *>(handle->data);
    self->Run();
  }

  uv_async_t _async;
  uv_loop_t* _loop;
  bool _should_stop = false;
  T& _target;
};

}  // namespace node_webrtc

#endif  // SRC_EVENTLOOP_H_
