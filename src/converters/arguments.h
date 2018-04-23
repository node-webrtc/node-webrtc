/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */

/*
 * This file defines functions for decomposing method arguments.
 */

#ifndef SRC_CONVERTERS_ARGUMENTS_H_
#define SRC_CONVERTERS_ARGUMENTS_H_

#include <tuple>

#include "nan.h"

#include "src/converters.h"
#include "src/functional/curry.h"
#include "src/functional/validation.h"

namespace node_webrtc {

struct Arguments {
  Nan::NAN_METHOD_ARGS_TYPE info;
  explicit Arguments(Nan::NAN_METHOD_ARGS_TYPE info): info(info) {}
};

template <typename A>
struct Converter<Arguments, A> {
  static Validation<A> Convert(Arguments args) {
    return From<A>(args.info[0]);
  }
};

template <typename L, typename R>
struct Converter<Arguments, Either<L, R>> {
  static Validation<Either<L, R>> Convert(Arguments args) {
    return From<L>(args).Map(&Either<L, R>::Left)
        | (From<R>(args).Map(&Either<L, R>::Right));
  }
};

template <typename A, typename B>
static std::tuple<A, B> Make2Tuple(A a, B b) {
  return std::make_tuple(a, b);
}

template <typename A, typename B>
struct Converter<Arguments, std::tuple<A, B>> {
  static Validation<std::tuple<A, B>> Convert(Arguments args) {
    return curry(Make2Tuple<A, B>)
        % From<A>(args.info[0])
        * From<B>(args.info[1]);
  }
};

template <typename A, typename B, typename C>
static std::tuple<A, B> Make3Tuple(A a, B b, C c) {
  return std::make_tuple(a, b, c);
}

template <typename A, typename B, typename C>
struct Converter<Arguments, std::tuple<A, B, C>> {
  static Validation<std::tuple<A, B, C>> Convert(Arguments args) {
    return curry(Make3Tuple<A, B, C>)
        % From<A>(args.info[0])
        * From<B>(args.info[1])
        * From<C>(args.info[2]);
  }
};

}  // namespace node_webrtc

#endif  // SRC_CONVERTERS_ARGUMENTS_H_
