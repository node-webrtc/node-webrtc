#pragma once

#include <atomic>
#include <mutex>

#include <node-addon-api/napi.h>

#include "src/node/napi_async_object_wrap.h"
#include "src/node/napi_event_loop.h"

namespace node_webrtc {

namespace napi {

template <typename T>
class AsyncObjectWrapWithLoop
  : public AsyncObjectWrap<T>
  , public EventLoop<T> {
 public:
  AsyncObjectWrapWithLoop(
      const char* name,
      T& target,
      const Napi::CallbackInfo& info) :
    AsyncObjectWrap<T>(name, info),
    EventLoop<T>(info.Env(), name, target) {
    this->Ref();
  }

 protected:
  /**
   * This method will be invoked once the AsyncObjectWrapWithLoop stops.
   */
  void DidStop() override {
    this->Unref();
  }
};

}  // namespace napi

}  // namespace node_webrtc
