/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_ASYNCOBJECTWRAP_H_
#define SRC_ASYNCOBJECTWRAP_H_

#include <atomic>

#include "nan.h"
#include "uv.h"

namespace node_webrtc {

/**
 * AsyncObjectWrap combines AsyncResource and ObjectWrap. You cannot subclass both AsyncResource and ObjectWrap, as this
 * will cause issues when the AsyncResource emits its "destroy" event. So, instead, AsyncObjectWrap subclasses
 * ObjectWrap and contains an AsyncResource member. AsyncObjectWraps support reference counting.
 */
class AsyncObjectWrap: private Nan::ObjectWrap {
 public:
  /**
   * Construct an AsyncObjectWrap. The name you provide is the name of the AsyncResource.
   * @param name the name of the AsyncResource
   */
  explicit AsyncObjectWrap(const char* name) {
    _async_resource = new Nan::AsyncResource(name);
    uv_mutex_init(&_async_resource_lock);
  }

  virtual ~AsyncObjectWrap() override {
    DestroyAsyncResource();
    uv_mutex_destroy(&_async_resource_lock);
  }

  /**
   * Increment the reference count.
   */
  inline void AddRef() {
    if (_reference_count.fetch_add(1) == 0) {
      Ref();
    }
  }

  /**
   * Decrement the reference count.
   */
  inline void RemoveRef() {
    Nan::HandleScope scope;
    if (_reference_count.fetch_sub(1) == 1) {
      DestroyAsyncResource();
      Unref();
    }
  }

  /**
   * Convert the AsyncObjectWrap to an Object.
   * @return object the Object
   */
  inline v8::Local<v8::Object> ToObject() {
    Nan::EscapableHandleScope scope;
    return scope.Escape(handle());
  }

  /**
   * Convert an Object to an AsyncObjectWrap.
   * @tparam T the AsyncObjectWrap type
   * @param object the Object to convert
   * @return asyncObjectWrap an AsyncObjectWrap
   */
  template <typename T>
  inline static T* Unwrap(v8::Local<v8::Object> object) {
    auto unwrapped = Nan::ObjectWrap::Unwrap<Nan::ObjectWrap>(object);
    return reinterpret_cast<T*>(unwrapped);
  }

  /**
   * Wrap an Object.
   * @param object the Object to wrap
   */
  inline void Wrap(v8::Local<v8::Object> object) {
    Nan::ObjectWrap::Wrap(object);
  }

 protected:
  /**
   * Make a callback.
   * @param name the name of the callback
   * @param argc the number of arguments
   * @param argv the arguments
   */
  inline void MakeCallback(const char* name, const int argc, v8::Local<v8::Value>* argv) {
    uv_mutex_lock(&_async_resource_lock);
    if (_async_resource) {
      _async_resource->runInAsyncScope(ToObject(), name, argc, argv);
    }
    uv_mutex_unlock(&_async_resource_lock);
  }

 private:
  Nan::AsyncResource* _async_resource;
  uv_mutex_t _async_resource_lock;
  std::atomic_int _reference_count = {0};

  /**
   * Destroy the AsyncResource.
   */
  inline void DestroyAsyncResource() {
    uv_mutex_lock(&_async_resource_lock);
    if (_async_resource) {
      delete _async_resource;
      _async_resource = nullptr;
    }
    uv_mutex_unlock(&_async_resource_lock);
  }
};

}  // namespace node_webrtc

#endif  // SRC_ASYNCOBJECTWRAP_H_
