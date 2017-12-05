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
class EventLoop: EventQueue<T> {
 public:
  explicit EventLoop(T& target)
  : EventQueue<T>()
  , _async(std::unique_ptr<uv_async_t>(new uv_async_t()))
  , _loop(uv_default_loop())
  , _target(target) {
    uv_async_init(_loop, _async.get(), [](uv_async_t* handle) {
      auto self = reinterpret_cast<EventLoop<T>*>(handle->data);
      self->Run();
    });
    _async->data = this;
  }

  ~EventLoop() {
    // NOTE(mroberts): We can't release this->_async; libuv still needs it.
  }

  /**
   * Dispatch an event to the EventLoop.
   * @param event the event to dispatch
   */
  void Dispatch(std::unique_ptr<Event<T>> event) {
    if (!this->stopped()) {
      this->Enqueue(std::move(event));
      uv_async_send(_async.get());
    }
  }

  /**
   * Stop the EventLoop.
   */
  void Stop() {
    if (!this->stopped()) {
      this->_should_stop = true;
      uv_async_send(_async.get());
    }
  }

  /**
   * Returns true if Stop was called.
   */
  bool stopped() const {
    return this->_should_stop;
  }

 protected:
  virtual void Run() {
    if (!this->stopped()) {
      while (auto event = this->Dequeue()) {
        if (!this->stopped()) {
          event->Dispatch(this->_target);
        }
        if (this->stopped()) {
          break;
        }
      }
    }
    if (this->stopped() && this->_async) {
      uv_close(reinterpret_cast<uv_handle_t*>(this->_async.release()), [](uv_handle_t* handle) {
        delete handle;
      });
    }
  }

 private:
  std::unique_ptr<uv_async_t> _async;
  uv_loop_t* _loop;
  bool _should_stop = false;
  T& _target;
};

}  // namespace node_webrtc

#endif  // SRC_EVENTLOOP_H_
