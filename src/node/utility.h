/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include "src/converters.h"
#include "src/converters/napi.h"

#define CREATE_DEFERRED(E, D) \
  auto deferred = Napi::Promise::Deferred::New(E);

#define RETURNS_PROMISE(R) \
  CREATE_RESOLVER(R) \
  info.GetReturnValue().Set(R->GetPromise());

namespace node_webrtc {

template <typename T>
bool Resolve(Napi::Promise::Deferred deferred, T input) {
  auto env = deferred.Env();
  auto maybeOutput = From<Napi::Value>(std::make_pair(env, input));
  if (maybeOutput.IsValid()) {
    deferred.Resolve(maybeOutput.UnsafeFromValid());
    return true;
  }
  deferred.Reject(Napi::Error::New(env, maybeOutput.ToErrors()[0]).Value());
  return false;
}

template <typename T>
bool Reject(Napi::Promise::Deferred deferred, T input) {
  auto env = deferred.Env();
  auto maybeOutput = From<Napi::Value>(std::make_pair(env, input));
  if (maybeOutput.IsValid()) {
    deferred.Reject(maybeOutput.UnsafeFromValid());
    return true;
  }
  deferred.Reject(Napi::Error::New(env, maybeOutput.ToErrors()[0]).Value());
  return false;
}

}  // namespace node_webrtc
