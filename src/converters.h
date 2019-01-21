/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
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

/**
 * This macro declares a node_webrtc::Converter from I to O.
 *
 * @param I the input type
 * @param O the output type
 */
#define DECLARE_CONVERTER(I, O) \
  template <> \
  struct Converter<I, O> { \
    static Validation<O> Convert(I); \
  };

/**
 * This macro declares a node_webrtc::Converter from T to v8::Local<v8::Value>.
 *
 * @param T the input type
 */
#define DECLARE_TO_JS(T) DECLARE_CONVERTER(T, v8::Local<v8::Value>)

/**
 * This macro declares a node_webrtc::Converter from v8::Local<v8::Value> to T.
 *
 * @param T the output type
 */
#define DECLARE_FROM_JS(T) DECLARE_CONVERTER(v8::Local<v8::Value>, T)

/**
 * This macro declares node_webrtc::Converter instances between T and v8::Local<v8::Value>.
 *
 * @param T the type to convert
 */
#define DECLARE_TO_AND_FROM_JS(T) \
  DECLARE_TO_JS(T) \
  DECLARE_FROM_JS(T)

/**
 * This macro simplifies defining a node_webrtc::Converter from I to O.
 *
 * @param I the input type
 * @param O the output type
 * @param V the name of the input variable to convert
 */
#define CONVERTER_IMPL(I, O, V) node_webrtc::Validation<O> node_webrtc::Converter<I, O>::Convert(I V)

/**
 * This macro simplifies defining a node_webrtc::Converter from T to v8::Local<v8::Value>.
 *
 * @param T the input type
 * @param V the name of the input variable to convert
 */
#define TO_JS_IMPL(T, V) CONVERTER_IMPL(T, v8::Local<v8::Value>, V)

/**
 * This macro simplifies defining a node_webrtc::Converter from v8::Local<v8::Value> to T.
 *
 * @param T the output type
 * @param V the name of the input variable to convert
 */
#define FROM_JS_IMPL(T, V) CONVERTER_IMPL(v8::Local<v8::Value>, T, V)

/**
 * This macro defines a node_webrtc::Converter from I to O when node_webrtc::Converter instances from
 *
 * 1. I to M, and
 * 2. M to O
 *
 * exist.
 *
 * @param I the input type
 * @param M the intermediate type
 * @param O the output type
 */
#define CONVERT_VIA(I, M, O) \
  CONVERTER_IMPL(I, O, value) { \
    return node_webrtc::Converter<I, M>::Convert(value).FlatMap<O>(node_webrtc::Converter<M, O>::Convert); \
  }
// TODO(mroberts): This stuff is not so general as to warrant inclusion in converters.h.


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
    return From<L>(s).Map(&MakeLeft<R, L>)
        | (From<R>(s).Map(&MakeRight<L, R>));
  }
};

}  // namespace node_webrtc

#endif  // SRC_CONVERTERS_H_
