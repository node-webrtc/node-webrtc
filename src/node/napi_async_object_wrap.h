#pragma once

#include <initializer_list>
#include <mutex>

#include <node-addon-api/napi.h>

namespace node_webrtc {

namespace napi {

template <typename T>
class AsyncObjectWrap: public Napi::ObjectWrap<T> {
 private:
  Napi::AsyncContext* _async_context;
  std::mutex _async_context_mutex;
  std::atomic_int _reference_count = {0};

  void DestroyAsyncContext() {
    _async_context_mutex.lock();
    if (_async_context) {
      delete _async_context;
      _async_context = nullptr;
    }
    _async_context_mutex.unlock();
  }

 public:
  AsyncObjectWrap(
      const char* name,
      const Napi::CallbackInfo& info):
    Napi::ObjectWrap<T>(info) {
    this->_async_context = new Napi::AsyncContext(info.Env(), name, this->Value());
  }

  virtual ~AsyncObjectWrap() {
    DestroyAsyncContext();
  }

  void Ref() {
    if (_reference_count.fetch_add(1) == 0) {
      Napi::ObjectWrap<T>::Ref();
    }
  }

  void Unref() {
    if (_reference_count.fetch_sub(1) == 1) {
      Napi::ObjectWrap<T>::Unref();
    }
  }

 protected:
  void MakeCallback(const char* name, const std::initializer_list<napi_value>& args) {
    auto self = this->Value();
    auto maybeFunction = self.Get(name);
    if (maybeFunction.IsFunction()) {
      _async_context_mutex.lock();
      if (_async_context) {
        napi_async_context context = *_async_context;
        maybeFunction.template As<Napi::Function>().MakeCallback(self, args, context);
      }
      _async_context_mutex.unlock();
    }
  }
};

}  // namespace napi

}  // namespace node_webrtc
