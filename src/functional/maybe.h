/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
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

#ifndef SRC_FUNCTIONAL_MAYBE_H_
#define SRC_FUNCTIONAL_MAYBE_H_

#include <cassert>
#include <functional>
#include <type_traits>

namespace node_webrtc {

/**
 * Maybe represents an optional value.
 * @tparam T the value type
 */
template <typename T>
class Maybe {
 public:
  /**
   * Construct an empty Maybe.
   */
  Maybe(): _is_just(false), _value(T()) {}

  /**
   * Construct a non-empty Maybe.
   * @param value the value to inject into the Maybe
   */
  explicit Maybe(const T& value): _is_just(true), _value(value) {}

  /**
   * Maybe forms an applicative. Apply a Maybe.
   * @tparam F the type of a function from T to S
   * @param f a Maybe of a function from T to S
   * @return the result of applying the Maybe
   */
  template <typename F>
  Maybe<typename std::result_of<F(T)>::type> Apply(const Maybe<F> f) const {
    return f.IsJust() && _is_just ? Maybe(f.UnsafeFromJust()(_value)) : Nothing();
  }

  /**
   * Eliminate a Maybe. You must provide a default value to handle the empty
   * case.
   * @param default_value the default value to use in the empty case
   * @return the value in the Maybe, if non-empty; otherwise, the default value
   */
  T FromMaybe(T default_value) const {
    return _is_just ? _value : default_value;
  }

  /**
   * Check whether or not the Maybe is non-empty.
   * @return true if the Maybe is non-empty; otherwise, false
   */
  bool IsJust() const {
    return _is_just;
  }

  /**
   * Check whether or not the Maybe is empty.
   * @return true if the Maybe is empty; otherwise, false
   */
  bool IsNothing() const {
    return !_is_just;
  }

  /**
   * Maybe forms a functor. Map a function over Maybe.
   * @tparam F the type of a function from T to S
   * @param f a function from T to S
   * @return the mapped Maybe
   */
  template <typename F>
  Maybe<typename std::result_of<F(T)>::type> Map(F f) const {
    return _is_just
        ? Maybe<typename std::result_of<F(T)>::type>::Just(f(_value))
        : Maybe<typename std::result_of<F(T)>::type>::Nothing();
  }

  /**
   * Maybe forms an alternative. If "this" is non-empty, return this; otherwise, that
   * @param that another Maybe
   * @return this or that
   */
  Maybe<T> Or(const Maybe<T>& that) const {
    return _is_just ? this : that;
  }

  /**
   * If "this" contains a value, return it; otherwise, compute a value and
   * return it
   * @param compute
   * @return
   */
  T Or(std::function<T()> compute) const {
    return _is_just ? _value : compute();
  }

  /**
   * Unsafely eliminate a Maybe. This only works if the Maybe is non-empty.
   * @return the value in the Maybe, if non-empty; otherwise, undefined
   */
  T UnsafeFromJust() const {
    assert(_is_just);
    return _value;
  }

  /**
   * Construct an empty Maybe.
   * @return an empty Maybe
   */
  static Maybe<T> Nothing() {
    return Maybe();
  }

  /**
   * Construct a non-empty Maybe.
   * @param value the value present in the Maybe
   * @return a non-empty Maybe
   */
  static Maybe<T> Just(const T& value) {
    return Maybe(value);
  }

 private:
  bool _is_just;
  T _value;
};

}  // namespace node_webrtc

#endif  // SRC_FUNCTIONAL_MAYBE_H_
