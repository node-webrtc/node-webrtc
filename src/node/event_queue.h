/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <memory>
#include <mutex>
#include <queue>

#include "events.h"

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
    _mutex.lock();
    _events.push(std::move(event));
    _mutex.unlock();
  }

  /**
   * Attempt to dequeue an Event. If the EventQueue is empty, this method
   * returns nullptr.
   * @return the dequeued Event or nullptr
   */
  std::unique_ptr<Event<T>> Dequeue() {
    _mutex.lock();
    if (_events.empty()) {
      _mutex.unlock();
      return nullptr;
    }
    auto event = std::move(_events.front());
    _events.pop();
    _mutex.unlock();
    return event;
  }

  virtual ~EventQueue() = default;

 private:
  std::queue<std::unique_ptr<Event<T>>> _events;
  std::mutex _mutex{};
};

}  // namespace node_webrtc
