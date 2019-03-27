/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <iosfwd>
#include <memory>

#include <nan.h>
#include <v8.h>

#include "src/converters/dictionaries.h"
#include "src/converters/v8.h"
#include "src/error.h"
#include "src/functional/either.h"

namespace node_webrtc {

/**
 * Event represents an event that can be dispatched to a target.
 * @tparam T the target type
 */
template<typename T>
class Event {
 public:
  /**
   * Dispatch the Event to the target.
   * @param target the target to dispatch to
   */
  virtual void Dispatch(T&) {
    // Do nothing.
  }

  virtual ~Event() = default;

  static std::unique_ptr<Event<T>> Create() {
    return std::unique_ptr<Event<T>>(new Event<T>());
  }
};

template <typename F, typename T>
class Callback: public Event<T> {
 public:
  void Dispatch(T&) override {
    _callback();
  }

  static std::unique_ptr<Callback<F, T>> Create(F callback) {
    return std::unique_ptr<Callback<F, T>>(new Callback(std::move(callback)));
  }

 private:
  explicit Callback(F callback): _callback(std::move(callback)) {}
  F _callback;
};

template <typename T, typename F>
static std::unique_ptr<Callback<F, T>> CreateCallback(F callback) {
  return Callback<F, T>::Create(std::move(callback));
}

template <typename T>
class Callback1: public Event<T> {
 public:
  void Dispatch(T& target) override {
    _callback(target);
  }

  static std::unique_ptr<Callback1<T>> Create(std::function<void(T&)> callback) {
    return std::unique_ptr<Callback1<T>>(new Callback1(std::move(callback)));
  }

 private:
  explicit Callback1(std::function<void(T&)> callback): _callback(std::move(callback)) {}
  const std::function<void(T&)> _callback;
};

/**
 * A PromiseEvent can be dispatched to a PromiseFulfillingEventLoop in order to
 * resolve or reject a Promise.
 * @tparam T the PromiseFulfillingEventLoop type
 * @tparam L the type of values representing failure
 * @tparam R the type of values representing success
 */
template <typename T, typename R = Undefined, typename L = SomeError>
class PromiseEvent: public Event<T> {
 public:
  void Dispatch(T&) override {
    Nan::HandleScope scope;
    if (_resolver) {
      auto resolver = Nan::New(*_resolver);
      _result.template FromEither<void>([resolver](L error) {
        CONVERT_OR_REJECT_AND_RETURN(resolver, error, value, v8::Local<v8::Value>);
        resolver->Reject(Nan::GetCurrentContext(), value).IsNothing();
      }, [resolver](R result) {
        CONVERT_OR_REJECT_AND_RETURN(resolver, result, value, v8::Local<v8::Value>);
        resolver->Resolve(Nan::GetCurrentContext(), value).IsNothing();
      });
    }
  }

  void Reject(L error) {
    _result = Either<L, R>::Left(error);
  }

  void Resolve(R result) {
    _result = Either<L, R>::Right(result);
  }

  static std::pair<v8::Local<v8::Promise::Resolver>, std::unique_ptr<PromiseEvent<T, R, L>>> Create() {
    Nan::EscapableHandleScope scope;
    auto resolver = v8::Promise::Resolver::New(Nan::GetCurrentContext()).ToLocalChecked();
    auto event = std::unique_ptr<PromiseEvent<T, R, L>>(new PromiseEvent<T, R, L>(
                std::make_unique<Nan::Persistent<v8::Promise::Resolver>>(
                    resolver)));
    return std::pair<v8::Local<v8::Promise::Resolver>, std::unique_ptr<PromiseEvent<T, R, L>>>(
            scope.Escape(resolver),
            std::move(event));
  }

 protected:
  explicit PromiseEvent(std::unique_ptr<Nan::Persistent<v8::Promise::Resolver>> resolver)
    : _resolver(std::move(resolver)) {}

 private:
  std::unique_ptr<Nan::Persistent<v8::Promise::Resolver>> _resolver;
  Either<L, R> _result;
};

}  // namespace node_webrtc
