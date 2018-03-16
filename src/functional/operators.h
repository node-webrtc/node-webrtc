/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */

/*
 * This file defines operators that make it nicer to work with alternatives,
 * applicatives, and functors. They are modeled after FTL:
 * https://github.com/beark/ftl
 */

#ifndef SRC_FUNCTIONAL_OPERATORS_H_
#define SRC_FUNCTIONAL_OPERATORS_H_

namespace node_webrtc {

/**
 * Syntactic sugar for Or
 * @tparam T the type of some alternative
 * @tparam A some type A
 * @param left an alternative of a value of type A
 * @param right an alternative of a value of type A
 * @return an alternative of a value of type A
 */
template <template <typename> class T, typename A>
static T<A> operator|(const T<A> left, const T<A> right) {
  return left.Or(right);
}

/**
 * Syntactic sugar for Apply
 * @tparam T the type of some functor
 * @tparam F the type of a function from A to B
 * @tparam A some type A
 * @param f an applicative of a function from A to B
 * @param a a value of type A
 * @return an applicative of a value of type B
 */
template <template <typename> class T, typename F, typename A>
static T<typename std::result_of<F(A)>::type> operator*(const T<F> f, const T<A> a) {
  return a.Apply(f);
}

/**
 * Syntactic sugar for Map
 * @tparam T the type of some functor
 * @tparam F the type of a function from A to B
 * @tparam A some type A
 * @param f a function from A to B
 * @param a a value of type A
 * @return a functor of a value of type B
 */
template <template <typename> class T, typename F, typename A>
static T<typename std::result_of<F(A)>::type> operator%(const F f, const T<A> a) {
  return a.Map(f);
}

}  // namespace node_webrtc

#endif  // SRC_FUNCTIONAL_OPERATORS_H_
