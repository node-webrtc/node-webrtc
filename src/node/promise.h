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

#include "events.h"
#include "utility.h"

namespace node_webrtc {

template <typename T, typename F>
class Promise: public Event<T> {
 public:
  Promise(
      const std::shared_ptr<Nan::Persistent<v8::Promise::Resolver>>& resolver,
      F callback)
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
std::unique_ptr<Promise<T, F>> CreatePromise(v8::Local<v8::Promise::Resolver> resolver, F callback) {
  return std::make_unique<Promise<T, F>>(
          std::make_shared<Nan::Persistent<v8::Promise::Resolver>>(resolver),
          callback);
}

template <typename T>
class PromiseCreator {
 public:
  PromiseCreator(
      T* target,
      v8::Local<v8::Promise::Resolver> resolver)
    : _target(target)
    , _resolver(std::make_shared<Nan::Persistent<v8::Promise::Resolver>>(resolver)) {}

  template <typename F>
  void Dispatch(F callback) {
    _target->Dispatch(std::make_unique<Promise<T, F>>(std::move(_resolver), callback));
  }

  template <typename F>
  void Resolve(F value) {
    Dispatch([value](auto resolver) {
      node_webrtc::Resolve(resolver, value);
    });
  }

  template <typename F>
  void Reject(F value) {
    Dispatch([value](auto resolver) {
      node_webrtc::Reject(resolver, value);
    });
  }

 private:
  T* _target;
  std::shared_ptr<Nan::Persistent<v8::Promise::Resolver>> _resolver;
};

}  // namespace node_webrtc
