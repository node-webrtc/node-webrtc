/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <functional>
#include <memory>

namespace node_webrtc {

/**
 * Event represents an event that can be dispatched to a target.
 * @tparam T the target type
 */
template <typename T> class Event {
public:
  Event(const Event &) = delete;
  Event(Event &&) = delete;
  Event &operator=(const Event &) = delete;
  Event &operator=(Event &&) = delete;
  Event() = default;
  virtual ~Event() = default;

  /**
   * Dispatch the Event to the target.
   * @param target the target to dispatch to
   */
  virtual void Dispatch(T &) {
    // Do nothing.
  }

  static std::unique_ptr<Event<T>> Create() {
    return std::unique_ptr<Event<T>>(new Event<T>());
  }
};

template <typename F, typename T> class Callback : public Event<T> {
public:
  void Dispatch(T &) override { _callback(); }

  static std::unique_ptr<Callback<F, T>> Create(F callback) {
    return std::unique_ptr<Callback<F, T>>(new Callback(std::move(callback)));
  }

private:
  explicit Callback(F callback) : _callback(std::move(callback)) {}
  F _callback;
};

template <typename T, typename F>
static std::unique_ptr<Callback<F, T>> CreateCallback(F callback) {
  return Callback<F, T>::Create(std::move(callback));
}

template <typename T> class Callback1 : public Event<T> {
public:
  void Dispatch(T &target) override { _callback(target); }

  static std::unique_ptr<Callback1<T>>
  Create(std::function<void(T &)> callback) {
    return std::unique_ptr<Callback1<T>>(new Callback1(std::move(callback)));
  }

private:
  explicit Callback1(const std::function<void(T &)> &callback)
      : _callback(callback) {}
  std::function<void(T &)> _callback;
};

} // namespace node_webrtc
