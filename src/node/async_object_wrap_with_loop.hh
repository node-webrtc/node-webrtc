#pragma once

#include <atomic>
#include <cstdio>
#include <mutex>

#include <node-addon-api/napi.h>

#include "src/node/async_object_wrap.hh"
#include "src/node/event_loop.hh"

namespace node_webrtc {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"

template <typename T>
class AsyncObjectWrapWithLoop : public AsyncObjectWrap<T>, public EventLoop<T> {
public:
  AsyncObjectWrapWithLoop(const char *name, T &target,
                          const Napi::CallbackInfo &info)
      : AsyncObjectWrap<T>(name, info), EventLoop<T>(info.Env(),
                                                     this->context(), target) {
    // printf("ref-ed %p (async object constructor)\n", (void *)this); // NOLINT
    // this->Ref();
  }

protected:
  /**
   * This method will be invoked once the AsyncObjectWrapWithLoop stops.
   */
  void DidStop() override {
    // printf("unref-ed %p (async object DidStop)\n", (void *)this); // NOLINT
    // this->Unref();
  }
};

#pragma clang diagnostic pop

} // namespace node_webrtc
