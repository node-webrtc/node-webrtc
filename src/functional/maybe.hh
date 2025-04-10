/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */

/*
 * This file defines Maybe, modeled after the Haskell's:
 * https://hackage.haskell.org/package/base/docs/Data-Maybe.html
 */

#pragma once

#include <cassert>
#include <functional>
#include <optional>
#include <type_traits>

namespace node_webrtc {

/**
 * Maybe represents an optional value.
 * @tparam T the value type
 */
template <typename T> class Maybe {
public:
  /**
   * Construct an empty Maybe.
   */
  Maybe() : _value(std::nullopt) {}

  /**
   * Construct a non-empty Maybe.
   * @param value the value to inject into the Maybe
   */
  explicit Maybe(const T &value) : _value(value) {}

  /**
   * Maybe forms an applicative. Apply a Maybe.
   * @tparam F the type of a function from T to S
   * @param f a Maybe of a function from T to S
   * @return the result of applying the Maybe
   */
  template <typename F>
  Maybe<typename std::invoke_result_t<F, T>> Apply(const Maybe<F> f) const {
    return f.IsJust() && _value.has_value()
               ? Maybe<typename std::invoke_result_t<F, T>>::Just(
                     f.UnsafeFromJust()(_value.value()))
               : Maybe<typename std::invoke_result_t<F, T>>::Nothing();
  }

  /**
   * Maybe forms a Monad. FlatMap a Maybe.
   * @tparam S some type S
   * @param f a function from T to a Maybe of S
   * @return a Maybe of S
   */
  template <typename S> Maybe<S> FlatMap(std::function<Maybe<S>(T)> f) const {
    return _value.has_value() ? f(_value.value()) : Maybe<S>::Nothing();
  }

  /**
   * Eliminate a Maybe. You must provide a default value to handle the empty
   * case.
   * @param default_value the default value to use in the empty case
   * @return the value in the Maybe, if non-empty; otherwise, the default value
   */
  T FromMaybe(T default_value) const {
    return _value.has_value() ? _value.value() : default_value;
  }

  /**
   * Check whether or not the Maybe is non-empty.
   * @return true if the Maybe is non-empty; otherwise, false
   */
  [[nodiscard]] bool IsJust() const { return _value.has_value(); }

  /**
   * Check whether or not the Maybe is empty.
   * @return true if the Maybe is empty; otherwise, false
   */
  [[nodiscard]] bool IsNothing() const { return !_value.has_value(); }

  /**
   * Maybe forms a functor. Map a function over Maybe.
   * @tparam F the type of a function from T to S
   * @param f a function from T to S
   * @return the mapped Maybe
   */
  template <typename F>
  Maybe<typename std::invoke_result_t<F, T>> Map(F f) const {
    return _value.has_value()
               ? Maybe<typename std::invoke_result_t<F, T>>::Just(
                     f(_value.value()))
               : Maybe<typename std::invoke_result_t<F, T>>::Nothing();
  }

  /**
   * Maybe forms an alternative. If "this" is non-empty, return this; otherwise,
   * that
   * @param that another Maybe
   * @return this or that
   */
  Maybe<T> Or(const Maybe<T> &that) const {
    return _value.has_value() ? this : that;
  }

  /**
   * If "this" is non-empty, return this; otherwise, return a
   * default-constructed T
   */
  T OrDefault() const { return _value.has_value() ? _value.value() : T(); }

  /**
   * If "this" contains a value, return it; otherwise, compute a value and
   * return it
   * @param compute
   * @return
   */
  T Or(std::function<T()> compute) const {
    return _value.has_value() ? _value.value() : compute();
  }

  /**
   * Unsafely eliminate a Maybe. This only works if the Maybe is non-empty.
   * @return the value in the Maybe, if non-empty; otherwise, undefined
   */
  T UnsafeFromJust() const {
    assert(_value.has_value());
    return *_value;
  }

  /**
   * Construct an empty Maybe.
   * @return an empty Maybe
   */
  static Maybe<T> Nothing() { return Maybe(); }

  /**
   * Construct a non-empty Maybe.
   * @param value the value present in the Maybe
   * @return a non-empty Maybe
   */
  static Maybe<T> Just(const T &value) { return Maybe(value); }

private:
  std::optional<T> _value;
};

template <typename T> static Maybe<T> MakeJust(const T &t) {
  return Maybe<T>::Just(t);
}

template <typename T> static Maybe<T> MakeNothing() {
  return Maybe<T>::Nothing();
}

} // namespace node_webrtc
