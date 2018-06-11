/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_EVENTQUEUE_H_
#define SRC_EVENTQUEUE_H_

#include <uv.h>

#include <memory>
#include <queue>

#include "src/events.h"

namespace node_webrtc {

/**
 * EventQueue is a thread-safe Event queue. It allows you to enqueue events
 * from one thread and dequeue them from another (or the same).
 * @tparam T the Event target type
 */
template <typename T>
class EventQueue {
 public:
  /**
   * Enqueue an Event.
   * @param event the event to enqueue
   */
  void Enqueue(std::unique_ptr<Event<T>> event) {
    uv_mutex_lock(&_lock);
    _events.push(std::move(event));
    uv_mutex_unlock(&_lock);
  }

  /**
   * Attempt to dequeue an Event. If the EventQueue is empty, this method
   * returns nullptr.
   * @return the dequeued Event or nullptr
   */
  std::unique_ptr<Event<T>> Dequeue() {
    uv_mutex_lock(&_lock);
    if (_events.empty()) {
      uv_mutex_unlock(&_lock);
      return nullptr;
    }
    auto event = std::move(_events.front());
    _events.pop();
    uv_mutex_unlock(&_lock);
    return event;
  }

  virtual ~EventQueue() {
    uv_mutex_destroy(&_lock);
  }

 protected:
  EventQueue() {
    uv_mutex_init(&_lock);
  }

 private:
  std::queue<std::unique_ptr<Event<T>>> _events;
  uv_mutex_t _lock;
};

}  // namespace node_webrtc

#endif  // SRC_EVENTQUEUE_H_
