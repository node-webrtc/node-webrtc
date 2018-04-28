/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_EVENTLOOP_H_
#define SRC_EVENTLOOP_H_

#include <uv.h>

#include <atomic>
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
class EventLoop: private EventQueue<T> {
 public:
  /**
   * Dispatch an event to the EventLoop.
   * @param event the event to dispatch
   */
  void Dispatch(std::unique_ptr<Event<T>> event) {
    this->Enqueue(std::move(event));
    uv_async_send(&_async);
  }

  virtual ~EventLoop() = default;

  bool should_stop() const {
    return _should_stop;
  }

 protected:
  explicit EventLoop(T& target): EventQueue<T>(), _loop(uv_default_loop()), _target(target) {
    uv_async_init(_loop, &_async, [](uv_async_t* handle) {
      auto self = static_cast<EventLoop<T>*>(handle->data);
      self->Run();
    });
    _async.data = this;
  }

  /**
   * This method will be invoked once the EventLoop stops.
   */
  virtual void DidStop() {
    // Do nothing.
  }

  virtual void Run() {
    while (auto event = this->Dequeue()) {
      event->Dispatch(_target);
      if (_should_stop) {
        break;
      }
    }
    if (_should_stop) {
      uv_close(reinterpret_cast<uv_handle_t*>(&_async), [](uv_handle_t* handle) {
        auto self = static_cast<EventLoop<T>*>(handle->data);
        self->DidStop();
      });
    }
  }

  /**
   * Stop the EventLoop.
   */
  void Stop() {
    _should_stop = true;
    Dispatch(Event<T>::Create());
  }

 private:
  uv_async_t _async;
  uv_loop_t* _loop;
  std::atomic<bool> _should_stop = {false};
  T& _target;
};

}  // namespace node_webrtc

#endif  // SRC_EVENTLOOP_H_
