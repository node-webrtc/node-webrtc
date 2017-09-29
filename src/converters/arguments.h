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
#include "src/functional/validation.h"

namespace node_webrtc {

template <typename T>
static Validation<Maybe<T>> GetOptional(Nan::NAN_METHOD_ARGS_TYPE info, const size_t i) {
  auto value = info[i];
  if (value->IsUndefined()) {
    return Validation<Maybe<T>>::Valid(Maybe<T>::Nothing());
  }
  return From<T>(value).Map(&Maybe<T>::Just);
}

template <typename T>
static Validation<T> GetOptional(
    Nan::NAN_METHOD_ARGS_TYPE info,
    const size_t i,
    T default_value) {
  return GetOptional<T>(info, i).Map(
      [default_value](const Maybe<T> maybeT) { return maybeT.FromMaybe(default_value); });
}

}  // namespace node_webrtc

#endif  // SRC_CONVERTERS_ARGUMENTS_H_
