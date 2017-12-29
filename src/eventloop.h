/* Copyright (c) 2017 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_EVENTLOOP_H_
#define SRC_EVENTLOOP_H_

#include <memory>
#include <queue>

#include "uv.h"

#include "src/eventqueue.h"
#include "src/events.h"

namespace node_webrtc {

/**
 * EventLoop is a thread-safe Event loop. It allows you to dispatch events from
 * one thread and handle them in another (or the same).
 * @tparam T the Event target type
 */
template <typename T>
class EventLoop
: public std::enable_shared_from_this<EventLoop<T>>
, EventQueue<T> {
 public:
  static std::shared_ptr<EventLoop<T>> Create(T& target) {
    auto eventLoop = new EventLoop<T>(target);
    return eventLoop->shared_from_this();
  }

  /**
   * Dispatch an event to the EventLoop.
   * @param event the event to dispatch
   */
  void Dispatch(std::unique_ptr<Event<T>> event) {
    if (!stopped()) {
      this->Enqueue(std::move(event));
      uv_async_send(&_async);
    }
  }

  /**
   * Stop the EventLoop.
   */
  void Stop() {
    if (!stopped()) {
      _should_stop = true;
      uv_async_send(&_async);
    }
  }

  /**
   * Returns true if Stop was called.
   */
  bool stopped() const {
    return _should_stop;
  }

 protected:
  explicit EventLoop(T& target)
  : EventQueue<T>()
  , _loop(uv_default_loop())
  , _target(target) {
    uv_async_init(_loop, &_async, [](uv_async_t* handle) {
      auto data = reinterpret_cast<std::shared_ptr<EventLoop<T>>*>(handle->data);
      auto self = *data;
      self->Run();
    });
    _async.data = new std::shared_ptr<EventLoop<T>>(this);
  }

  virtual void Run() {
    if (!stopped()) {
      while (auto event = this->Dequeue()) {
        if (!stopped()) {
          event->Dispatch(_target);
        }
        if (stopped()) {
          break;
        }
      }
    }
    if (stopped()) {
      uv_close(reinterpret_cast<uv_handle_t*>(&_async), [](uv_handle_t* handle) {
        auto data = reinterpret_cast<std::shared_ptr<EventLoop<T>>*>(handle->data);
        data->reset();
        delete data;
      });
    }
  }

 private:
  uv_async_t _async;
  uv_loop_t* _loop;
  bool _should_stop = false;
  T& _target;
};

}  // namespace node_webrtc

#endif  // SRC_EVENTLOOP_H_
