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

#pragma once

#include <memory>

#include "src/functional/either.h"
#include "src/functional/operators.h"
#include "src/functional/validation.h"

namespace node_webrtc {

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
 * This macro simplifies defining a node_webrtc::Converter from I to O.
 *
 * @param I the input type
 * @param O the output type
 * @param V the name of the input variable to convert
 */
#define CONVERTER_IMPL(I, O, V) Validation<O> Converter<I, O>::Convert(I V)

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
    return Converter<I, M>::Convert(value).FlatMap<O>(Converter<M, O>::Convert); \
  }

// TODO(mroberts): This stuff is not so general as to warrant inclusion in converters.h.

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

template <typename T>
struct Converter<T*, std::shared_ptr<T>> {
  static Validation<std::shared_ptr<T>> Convert(T* t) {
    return Validation<std::shared_ptr<T>>(std::shared_ptr<T>(t));
  }
};

}  // namespace node_webrtc
