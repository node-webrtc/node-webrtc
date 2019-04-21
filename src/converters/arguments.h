/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */

/*
 * This file defines functions for decomposing method arguments.
 */

#pragma once

#include <tuple>

#include <node-addon-api/napi.h>

#include "src/converters.h"
#include "src/converters/macros.h"
#include "src/functional/curry.h"
#include "src/functional/either.h"
#include "src/functional/operators.h"
#include "src/functional/validation.h"

namespace node_webrtc {

#define CONVERT_ARGS_OR_THROW_AND_RETURN_NAPI(I, O, T) \
  auto NODE_WEBRTC_UNIQUE_NAME(validation) = From<detail::argument_type<void(T)>::type>(Arguments(I)); \
  if (NODE_WEBRTC_UNIQUE_NAME(validation).IsInvalid()) { \
    auto error = NODE_WEBRTC_UNIQUE_NAME(validation).ToErrors()[0]; \
    Napi::TypeError::New(I.Env(), error).ThrowAsJavaScriptException(); \
    return I.Env().Undefined(); \
  } \
  auto O = NODE_WEBRTC_UNIQUE_NAME(validation).UnsafeFromValid();

#define CONVERT_ARGS_OR_THROW_AND_RETURN_VOID_NAPI(I, O, T) \
  auto NODE_WEBRTC_UNIQUE_NAME(validation) = From<detail::argument_type<void(T)>::type>(Arguments(I)); \
  if (NODE_WEBRTC_UNIQUE_NAME(validation).IsInvalid()) { \
    auto error = NODE_WEBRTC_UNIQUE_NAME(validation).ToErrors()[0]; \
    Napi::TypeError::New(I.Env(), error).ThrowAsJavaScriptException(); \
    return; \
  } \
  auto O = NODE_WEBRTC_UNIQUE_NAME(validation).UnsafeFromValid();

#define CONVERT_ARGS_OR_REJECT_AND_RETURN_NAPI(D, I, O, T) \
  auto NODE_WEBRTC_UNIQUE_NAME(validation) = From<detail::argument_type<void(T)>::type>(Arguments(I)); \
  if (NODE_WEBRTC_UNIQUE_NAME(validation).IsInvalid()) { \
    auto error = NODE_WEBRTC_UNIQUE_NAME(validation).ToErrors()[0]; \
    D.Reject(Napi::TypeError::New(D.Env(), error).Value()); \
    return D.Promise(); \
  } \
  auto O = NODE_WEBRTC_UNIQUE_NAME(validation).UnsafeFromValid();

struct Arguments {
  const Napi::CallbackInfo& info;
  explicit Arguments(const Napi::CallbackInfo& info): info(info) {}
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
    return From<L>(args).Map(&MakeLeft<R, L>)
        | (From<R>(args).Map(&MakeRight<L, R>));
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
