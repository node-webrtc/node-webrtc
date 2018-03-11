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

#include "src/common.h"
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
    TRACE_CALL;
    this->Enqueue(std::move(event));
    uv_async_send(&_async);
    TRACE_END;
  }

 protected:
  explicit EventLoop(T& target): EventQueue<T>(), _loop(uv_default_loop()), _target(target) {
    uv_async_init(_loop, &_async, reinterpret_cast<uv_async_cb>(Run));
    _async.data = this;
  }

  /**
   * This method will be invoked once the EventLoop stops.
   */
  virtual void DidStop() {
    // Do nothing.
  }

  /**
   * Stop the EventLoop.
   */
  void Stop() {
    TRACE_CALL;
    _should_stop = true;
    TRACE_END;
  }

 private:
  uv_async_t _async;
  uv_loop_t* _loop;
  std::atomic<bool> _should_stop = {false};
  T& _target;

  static void Run(uv_async_t* handle, int) {
    TRACE_CALL;

    auto self = reinterpret_cast<EventLoop<T>*>(handle->data);

    while (auto event = self->Dequeue()) {
      TRACE("Got event");
      event->Dispatch(self->_target);
      if (self->_should_stop) {
        TRACE("Event set _should_stop");
        break;
      }
    }
    TRACE("No more events");

    if (self->_should_stop) {
      TRACE("Stopping");
      self->_async.data = nullptr;
      self->DidStop();
      uv_close(reinterpret_cast<uv_handle_t*>(&self->_async), nullptr);
    }

    TRACE_END;
  }
};

}  // namespace node_webrtc

#endif  // SRC_EVENTLOOP_H_