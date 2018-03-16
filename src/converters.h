/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */

/*
 * This file defines conversion functions and short-hand for converting between
 * different types. Conversions can fail, hence we use Validation.
 */

#ifndef SRC_CONVERTERS_H_
#define SRC_CONVERTERS_H_

#include "src/functional/curry.h"
#include "src/functional/either.h"
#include "src/functional/maybe.h"
#include "src/functional/operators.h"
#include "src/functional/validation.h"

namespace node_webrtc {

/**
 * A Converter converts values from some "source" type S to values of some
 * "target" type T.
 * @tparam S the source type
 * @tparam T the target type
 */
template <typename S, typename T>
struct Converter {};

/**
 * From is short-hand for invoking a particular Converter.
 * @tparam T the target type
 * @tparam S the source type
 * @param s the source value
 * @return the target value
 */
template <typename T, typename S>
static Validation<T> From(const S s) {
  return Converter<S, T>::Convert(s);
}

/**
 * There is an "identity" Converter between values of the same type T.
 * @tparam T the source and target type
 */
template <typename T>
struct Converter<T, T> {
  static Validation<T> Convert(const T t) {
    return Validation<T>(t);
  }
};

/**
 * There is a Converter that tries first one conversion, then another.
 * @tparam S the source type
 * @tparam L a target type L
 * @tparam R a target type R
 */
template <typename S, typename L, typename R>
struct Converter<S, Either<L, R>> {
  static Validation<Either<L, R>> Convert(const S s) {
    return From<L>(s).Map(&Either<L, R>::Left)
        | (From<R>(s).Map(&Either<L, R>::Right));
  }
};

}  // namespace node_webrtc

#endif  // SRC_CONVERTERS_H_
