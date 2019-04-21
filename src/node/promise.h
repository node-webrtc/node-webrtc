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

#include <node-addon-api/napi.h>

#include "events.h"
#include "utility.h"

namespace node_webrtc {

template <typename T, typename F>
class Promise: public Event<T> {
 public:
  Promise(
      Napi::Promise::Deferred& deferred,
      F callback)
    : _callback(callback),
      _deferred(deferred) {}

  void Dispatch(T&) override {
    _callback(_deferred);
  }

 private:
  F _callback;
  Napi::Promise::Deferred _deferred;
};

template <typename T, typename F>
std::unique_ptr<Promise<T, F>> CreatePromise(Napi::Promise::Deferred deferred, F callback) {
  return std::make_unique<Promise<T, F>>(deferred, callback);
}

template <typename T>
class PromiseCreator {
 public:
  PromiseCreator(
      T* target,
      Napi::Promise::Deferred deferred)
    : _target(target)
    , _deferred(deferred) {}

  template <typename F>
  void Dispatch(F callback) {
    _target->Dispatch(std::make_unique<Promise<T, F>>(_deferred, callback));
  }

  template <typename F>
  void Resolve(F value) {
    Dispatch([value](auto deferred) {
      node_webrtc::Resolve(deferred, value);
    });
  }

  template <typename F>
  void Reject(F value) {
    Dispatch([value](auto deferred) {
      node_webrtc::Reject(deferred, value);
    });
  }

 private:
  T* _target;
  Napi::Promise::Deferred _deferred;
};

}  // namespace node_webrtc
