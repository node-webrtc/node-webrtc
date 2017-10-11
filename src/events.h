/* Copyright (c) 2017 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_EVENTS_H_
#define SRC_EVENTS_H_

#include <string>

namespace node_webrtc {

class DataChannel;
class DataChannelObserver;

/**
 * Event represents an event that can be dispatched to a target.
 * @tparam T the target type
 */
template<typename T>
class Event {
 public:
  /**
   * Dispatch the Event to the target.
   * @param target the target to dispatch to
   */
  virtual void Dispatch(T& target) {
    // Do nothing.
  }
};

template <typename T>
class ErrorEvent: public Event<T> {
 public:
  const std::string msg;

  void Dispatch(T&) override {}

 protected:
  explicit ErrorEvent(const std::string&& msg): msg(msg) {}
};

template <typename T, typename S>
class StateEvent: public Event<T> {
 public:
  const S state;

 protected:
  explicit StateEvent(const S state): state(state) {}
};

}  // namespace node_webrtc

#endif  // SRC_EVENTS_H_
