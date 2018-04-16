/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */

/*
 * This file defines Either, a sum type, modeled after the Haskell's:
 * https://hackage.haskell.org/package/base/docs/Data-Either.html
 */

#ifndef SRC_FUNCTIONAL_EITHER_H_
#define SRC_FUNCTIONAL_EITHER_H_

#include <cassert>
#include <functional>

namespace node_webrtc {

/**
 * Either is a sum type. It either contains a value of type L ("left") or a
 * value of type R ("right").
 * @tparam L the left type
 * @tparam R the right type
 */
template <typename L, typename R>
class Either {
 public:
  // TODO(mroberts): This is no good.
  Either(): _is_right(false), _left(L()), _right(R()) {}

  /**
   * Either forms an applicative. Apply an Either.
   * @tparam F the type of a function from R to S
   * @param f an Either of a function from R to S
   * @return the result of applying the Either
   */
  template <typename F>
  Either<L, typename std::result_of<F(R)>::type> Apply(const Either<L, F> f) const {
    if (f.IsLeft()) {
      return Either::Left(f.UnsafeFromLeft());
    } else if (IsLeft()) {
      return Either::Left(_left);
    }
    return Either::Right(f.UnsafeFromRight()(_right));
  }

  /**
   * Check whether or not the Either is "left".
   * @return true if the Either is left; otherwise, false
   */
  bool IsLeft() const {
    return !_is_right;
  }

  /**
   * Check whether or not the Either is "right".
   * @return true if the Either is right; otherwise, false
   */
  bool IsRight() const {
    return _is_right;
  }

  /**
   * Eliminate an Either. You must provide two functions for both the left and
   * right cases that convert each value to some common type T.
   * @tparam T the common type to convert to
   * @param fromLeft a function that converts values of type L to T
   * @param fromRight a function that converts values of type R to T
   * @return a value of type T
   */
  template <typename T>
  T FromEither(std::function<T(const L)> fromLeft, std::function<T(const R)> fromRight) const {
    return _is_right ? fromRight(_right) : fromLeft(_left);
  }

  /**
   * Eliminate an Either. You must provide a default value to handle the "right"
   * case.
   * @param default_value the default value to use in the right case
   * @return the value in the Either, if "left"; otherwise, the default value
   */
  L FromLeft(const L default_value) const {
    return _is_right ? default_value : _left;
  }

  /**
   * Eliminate an Either. You must provide a default value to handle the "left"
   * case.
   * @param default_value the default value to use in the left case
   * @return the value in the Either, if "right"; otherwise, the default value
   */
  R FromRight(const R default_value) const {
    return _is_right ? _right : default_value;
  }

  /**
   * Either forms a functor. Map a function over Either.
   * @tparam F the type of a function from R to S
   * @param f a function from R to S
   * @return the mapped Either
   */
  template <typename F>
  Either<L, typename std::result_of<F(R)>::type> Map(F f) const {
    return _is_right ? Either(f(_right)) : this;
  }

  /**
   * Either forms an alternative. If "this" is "right", return this; otherwise, that
   * @param that another Either
   * @return this or that
   */
  Either<L, R> Or(const Either<L, R> that) const {
    return _is_right ? this : that;
  }

  /**
   * Unsafely eliminate an Either. This only works if the Either is "left".
   * @return the left value in the Either, if left; otherwise, undefined
   */
  L UnsafeFromLeft() const {
    assert(!_is_right);
    return _left;
  }

  /**
   * Unsafely eliminate an Either. This only works if the Either is "right".
   * @return the right value in the Either, if right; otherwise, undefined
   */
  R UnsafeFromRight() const {
    assert(_is_right);
    return _right;
  }

  /**
   * Construct a "left" Either.
   * @param left the value to inject into the Either
   * @return a left Either
   */
  static Either<L, R> Left(const L left) {
    return Either(false, left, R());
  }

  /**
   * Construct a "right" Either.
   * @param right the value to inject into the Either
   * @return a right Either
   */
  static Either<L, R> Right(const R right) {
    return Either(true, L(), right);
  }

 private:
  Either(bool is_right, const L left, const R right): _is_right(is_right), _left(left), _right(right) {}

  bool _is_right;
  L _left;
  R _right;
};

}  // namespace node_webrtc

#endif  // SRC_FUNCTIONAL_EITHER_H_
