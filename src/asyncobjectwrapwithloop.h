/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_ASYNCOBJECTWRAPWITHLOOP_H_
#define SRC_ASYNCOBJECTWRAPWITHLOOP_H_

#include <atomic>

#include "nan.h"
#include "uv.h"

#include "src/asyncobjectwrap.h"
#include "src/promisefulfillingeventloop.h"

namespace node_webrtc {

/**
 * AsyncObjectWrapWithLoop combines AsyncObjectWrap and PromiseFulfillingEventLoop. Since the class instance is
 * associated with an event loop, we should AddRef when we Wrap it, and RemoveRef when the event loop closes.
 * @tparam T the Event target type
 */
template <typename T>
class AsyncObjectWrapWithLoop
  : private AsyncObjectWrap
  , private PromiseFulfillingEventLoop<T> {
 public:
  /**
   * Construct an AsyncObjectWrapWithLoop. The name you provide is the name of the AsyncResource, and the target you
   * provide is the Event target.
   * @param name the name of the AsyncResource
   * @param target the Event target
   */
  AsyncObjectWrapWithLoop(const char* name, T& target): AsyncObjectWrap(name), PromiseFulfillingEventLoop<T>(target) {}

  virtual ~AsyncObjectWrapWithLoop() override = default;

  /**
   * Increment the reference count.
   */
  inline void AddRef() {
    AsyncObjectWrap::AddRef();
  }

  /**
   * Dispatch an event to the AsyncObjectWrapWithLoop.
   * @param event the event to dispatch
   */
  void Dispatch(std::unique_ptr<Event<T>> event) {
    PromiseFulfillingEventLoop<T>::Dispatch(std::move(event));
  }

  /**
   * Decrement the reference count.
   */
  inline void RemoveRef() {
    AsyncObjectWrap::RemoveRef();
  }

  /**
   * Convert the AsyncObjectWrapWithLoop to an Object.
   * @return object the Object
   */
  inline v8::Local<v8::Object> ToObject() {
    Nan::EscapableHandleScope scope;
    return scope.Escape(AsyncObjectWrap::ToObject());
  }

  /**
   * Convert an Object to an AsyncObjectWrapWithLoop.
   * @tparam T the AsyncObjectWrapWithLoop type
   * @param object the Object to convert
   * @return asyncObjectWrapWithLoop an AsyncObjectWrapWithLoop
   */
  inline static T* Unwrap(v8::Local<v8::Object> object) {
    return AsyncObjectWrap::Unwrap<T>(object);
  }

  /**
   * Wrap an Object. This also calls AddRef once; therefore, Wrap initializes the reference count to one.
   * @param object the Object to wrap
   */
  inline void Wrap(v8::Local<v8::Object> object) {
    AsyncObjectWrap::Wrap(object);
    AddRef();
  }

 protected:
  /**
   * Invoked when the event loop closes. This calls RemoveRef.
   */
  void DidStop() override {
    RemoveRef();
  }

  /**
   * Make a callback.
   * @param name the name of the callback
   * @param argc the number of arguments
   * @param argv the arguments
   */
  inline void MakeCallback(const char* name, const int argc, v8::Local<v8::Value>* argv) {
    AsyncObjectWrap::MakeCallback(name, argc, argv);
  }

  /**
   * Stop the event loop.
   */
  inline void Stop() {
    PromiseFulfillingEventLoop<T>::Stop();
  }
};

}  // namespace node_webrtc

#endif  // SRC_ASYNCOBJECTWRAPWITHLOOP_H_
