/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <memory>
#include <type_traits>

#include <nan.h>
#include <v8.h>

#include "src/events.h"

namespace node_webrtc {

template <typename T, typename F>
class Promise: public Event<T> {
 public:
  Promise(
      F callback,
      std::shared_ptr<Nan::Persistent<v8::Promise::Resolver>> resolver)
    : _callback(callback),
      _resolver(resolver) {}

  void Dispatch(T&) override {
    _callback(resolver());
  }

 private:
  v8::Local<v8::Promise::Resolver> resolver() {
    Nan::EscapableHandleScope scope;
    v8::Local<v8::Promise::Resolver> resolver = Nan::New(*_resolver);
    return scope.Escape(resolver);
  }

  F _callback;
  std::shared_ptr<Nan::Persistent<v8::Promise::Resolver>> _resolver;
};

template <typename T, typename F>
std::unique_ptr<Promise<T, F>> CreatePromise(std::shared_ptr<Nan::Persistent<v8::Promise::Resolver>> resolver, F callback) {
  return std::make_unique<Promise<T, F>>(callback, resolver);
}

template <typename T>
class PromiseCreator {
 public:
  PromiseCreator(T* target, v8::Local<v8::Promise::Resolver> resolver): _target(target) {
    Nan::HandleScope scope;
    _resolver = std::make_shared<Nan::Persistent<v8::Promise::Resolver>>(resolver);
  }

  template <typename F>
  void Dispatch(F callback) {
    // NOTE(mroberts): This is a little unsafe, as it can only be called once.
    _target->Dispatch(std::move(CreatePromise<T, F>(_resolver, callback)));
  }

 private:
  T* _target;
  std::shared_ptr<Nan::Persistent<v8::Promise::Resolver>> _resolver;
};

}  // namespace node_webrtc
