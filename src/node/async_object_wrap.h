/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <atomic>
#include <mutex>
#include <vector>

#include <nan.h>
#include <node-addon-api/napi.h>
#include <v8.h>

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
  explicit AsyncObjectWrap(const char* name);

  ~AsyncObjectWrap() override;

  /**
   * Increment the reference count.
   */
  void AddRef();

  /**
   * Decrement the reference count.
   */
  void RemoveRef();

  /**
   * Convert the AsyncObjectWrap to an Object.
   * @return object the Object
   */
  v8::Local<v8::Object> ToObject();

  /**
   * Convert an Object to an AsyncObjectWrap.
   * @tparam T the AsyncObjectWrap type
   * @param object the Object to convert
   * @return asyncObjectWrap an AsyncObjectWrap
   */
  template <typename T>
  static T* Unwrap(v8::Local<v8::Object> object) {
    auto unwrapped = Nan::ObjectWrap::Unwrap<Nan::ObjectWrap>(object);
    return reinterpret_cast<T*>(unwrapped);
  }

  /**
   * Wrap an Object.
   * @param object the Object to wrap
   */
  void Wrap(v8::Local<v8::Object> object);

 protected:
  /**
   * Make a callback.
   * @param name the name of the callback
   * @param argc the number of arguments
   * @param argv the arguments
   */
  void MakeCallback(const char* name, int argc, v8::Local<v8::Value>* argv);

 private:
  Nan::AsyncResource* _async_resource;
  std::mutex _async_resource_mutex;
  std::atomic_int _reference_count = {0};

  /**
   * Destroy the AsyncResource.
   */
  void DestroyAsyncResource();
};

namespace napi {

template <typename T>
class AsyncObjectWrap
  : public Napi::ObjectWrap<T>
  , public Napi::AsyncContext {
 public:
  explicit AsyncObjectWrap(
      const char* name,
      const Napi::CallbackInfo& info):
    Napi::ObjectWrap<T>(info),
    Napi::AsyncContext(info.Env(), name, this->Value()) {}

  void AddRef() {
    this->Ref();
  }

  void RemoveRef() {
    this->Unref();
  }

 protected:
  void MakeCallback(const char* name, std::vector<Napi::Value> args) {
    auto self = this->Value();
    auto maybeFunction = self.Get(name);
    if (maybeFunction.IsFunction()) {
      maybeFunction.template As<Napi::Function>().Call(self, args);
    }
  }
};

}  // namespace napi

}  // namespace node_webrtc
