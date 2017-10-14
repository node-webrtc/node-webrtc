/* Copyright (c) 2017 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_EVENTS_PROMISE_H_
#define SRC_EVENTS_PROMISE_H_

#include <memory>

#include "nan.h"
#include "v8.h"

#include "src/events.h"

namespace node_webrtc {

/**
 * A PromiseEvent is can be dispatched to a PromiseFulfillingEventLoop in order
 * to resolve or reject a Promise.
 * @tparam T the PromiseFulfillingEventLoop type
 */
template <typename T>
class PromiseEvent: public Event<T> {
 public:
  /**
   * By default, a PromiseEvent just resolves its Promise with undefined.
   */
  virtual void Dispatch(T&) override {
    Nan::HandleScope scope;
    if (_resolver) {
      auto resolver = (*_resolver).Get(Nan::GetCurrentContext()->GetIsolate());
      resolver->Resolve(Nan::Undefined());
    }
  }

 protected:
  explicit PromiseEvent(std::unique_ptr<Nan::Persistent<v8::Promise::Resolver>> resolver)
  : _resolver(std::move(resolver)) {}

  std::unique_ptr<Nan::Persistent<v8::Promise::Resolver>> _resolver;
};

/**
 * A PromiseRejectionEvent is a PromiseEvent that, once dispatched to a
 * PromiseFulfillingEventLoop, rejects its Promise with an Error.
 * @tparam T the PromiseFulfillingEventLoop type
 */
template <typename T>
class PromiseRejectionEvent: public PromiseEvent<T> {
 public:
  /**
   * By default, a PromiseRejectionEvent just resolves its Promise with an
   * Error.
   */
  void Dispatch(T&) override {
    Nan::HandleScope scope;
    auto resolver = (*this->_resolver).Get(Nan::GetCurrentContext()->GetIsolate());
    resolver->Reject(Nan::Error(Nan::New(_reason).ToLocalChecked()));
  }

 protected:
  explicit PromiseRejectionEvent(
      std::unique_ptr<Nan::Persistent<v8::Promise::Resolver>> resolver,
      const std::string& reason)
      : PromiseEvent<T>(std::move(resolver))
      , _reason(reason) {}

 private:
  const std::string _reason;
};

}  // namespace node_webrtc

#endif  // SRC_EVENTS_PROMISE_H_
