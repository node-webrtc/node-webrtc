/* Copyright (c) 2017 The node-webrtc project authors. All rights reserved.
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

#include <nan.h>

#include "src/converters.h"
#include "src/functional/curry.h"
#include "src/functional/validation.h"

namespace node_webrtc {

template <typename A>
struct Converter<Nan::NAN_METHOD_ARGS_TYPE, A> {
  static Validation<A> Convert(Nan::NAN_METHOD_ARGS_TYPE info) {
    return From<A>(info[0]);
  }
};

template <typename A, typename B>
struct Converter<Nan::NAN_METHOD_ARGS_TYPE, std::tuple<A, B>> {
  static Validation<std::tuple<A, B>> Convert(Nan::NAN_METHOD_ARGS_TYPE info) {
    return curry(std::make_tuple<A, B>)
        % From<A>(info[0])
        * From<B>(info[1]);
  }
};

template <typename A, typename B, typename C>
struct Converter<Nan::NAN_METHOD_ARGS_TYPE, std::tuple<A, B, C>> {
  static Validation<std::tuple<A, B, C>> Convert(Nan::NAN_METHOD_ARGS_TYPE info) {
    return curry(std::make_tuple<A, B>)
        % From<A>(info[0])
        * From<B>(info[1])
        * From<C>(info[2]);
  }
};

}  // namespace node_webrtc

#endif  // SRC_CONVERTERS_ARGUMENTS_H_
