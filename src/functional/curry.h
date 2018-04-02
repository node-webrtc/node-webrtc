/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */

/*
 * This file defines a function for currying functions. It is borrowed from
 * https://stackoverflow.com/a/26768388
 */

#ifndef SRC_FUNCTIONAL_CURRY_H_
#define SRC_FUNCTIONAL_CURRY_H_

#include <functional>

namespace _dtl {

template <typename FUNCTION>
struct _curry;

// specialization for functions with a single argument
template <typename R, typename T>
struct _curry<std::function<R(T)>> {
  using type = std::function<R(T)>;

  const type result;

  _curry(type fun): result(fun) {}
};

// recursive specialization for functions with more arguments
template <typename R, typename T, typename...Ts> struct
_curry<std::function<R(T, Ts...)>> {
  using remaining_type = typename _curry<std::function<R(Ts...)>>::type;

  using type = std::function<remaining_type(T)>;

  const type result;

  _curry(std::function<R(T, Ts...)> fun): result(
        [ = ](const T & t) {
    return _curry<std::function<R(Ts...)>>(
    [ = ](const Ts & ...ts) {
      return fun(t, ts...);
    }
        ).result;
  }
  ) {}
};

}

template <typename R, typename...Ts>
auto curry(const std::function<R(Ts...)> fun) -> typename _dtl::_curry<std::function<R(Ts...)>>::type {
  return _dtl::_curry<std::function<R(Ts...)>>(fun).result;
}

template <typename R, typename...Ts>
auto curry(R(* const fun)(Ts...)) -> typename _dtl::_curry<std::function<R(Ts...)>>::type {
  return _dtl::_curry<std::function<R(Ts...)>>(fun).result;
}

#endif  // SRC_FUNCTIONAL_CURRY_H_
