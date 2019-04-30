#pragma once

#include <mutex>
#include <queue>

#include <node-addon-api/napi.h>

#include "src/node/deferrer.h"

namespace node_webrtc {

class AsyncContextReleaser
  : public Napi::ObjectWrap<AsyncContextReleaser>
  , private Deferrer {
 public:
  AsyncContextReleaser(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<AsyncContextReleaser>(info)
    , Deferrer(info.Env()) {}

  static AsyncContextReleaser* GetDefault();
  static void Init(Napi::Env, Napi::Object);

  void Release(Napi::AsyncContext*);

 protected:
  void Execute(Napi::Env);

 private:
  static AsyncContextReleaser* _default;
  static Napi::FunctionReference& constructor();

  std::queue<Napi::AsyncContext*> _contexts;
  std::mutex _contexts_mutex{};
};

}  // namespace node_webrtc
