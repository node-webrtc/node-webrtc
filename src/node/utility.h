/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <nan.h>
#include <v8.h>

#include "src/converters.h"
#include "src/converters/v8.h"

#define CREATE_RESOLVER(R) \
  auto R = v8::Promise::Resolver::New(Nan::GetCurrentContext()).ToLocalChecked();

#define RETURNS_PROMISE(R) \
  CREATE_RESOLVER(R) \
  info.GetReturnValue().Set(R->GetPromise());

namespace node_webrtc {

template<typename T>
bool Resolve(v8::Local<v8::Promise::Resolver> resolver, T input) {
  Nan::HandleScope scope;
  auto maybeOutput = node_webrtc::From<v8::Local<v8::Value>>(input);
  if (maybeOutput.IsValid()) {
    resolver->Resolve(Nan::GetCurrentContext(), maybeOutput.UnsafeFromValid()).IsNothing();
    return true;
  }
  auto error = Nan::New(maybeOutput.ToErrors()[0]).ToLocalChecked();
  resolver->Reject(Nan::GetCurrentContext(), error).IsNothing();
  return false;
}

template <typename T>
bool Reject(v8::Local<v8::Promise::Resolver> resolver, T input) {
  Nan::HandleScope scope;
  auto maybeOutput = node_webrtc::From<v8::Local<v8::Value>>(input);
  if (maybeOutput.IsValid()) {
    resolver->Reject(Nan::GetCurrentContext(), maybeOutput.UnsafeFromValid()).IsNothing();
    return true;
  }
  auto error = Nan::New(maybeOutput.ToErrors()[0]).ToLocalChecked();
  resolver->Reject(Nan::GetCurrentContext(), error).IsNothing();
  return false;
}

}  // namespace node_webrtc
