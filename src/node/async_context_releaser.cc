#include "src/node/async_context_releaser.h"

namespace node_webrtc {

AsyncContextReleaser* AsyncContextReleaser::_default = nullptr;

Napi::FunctionReference& AsyncContextReleaser::constructor() {
  static Napi::FunctionReference constructor;
  return constructor;
}

void AsyncContextReleaser::Release(Napi::AsyncContext* context) {
  _contexts_mutex.lock();
  _contexts.push(context);
  _contexts_mutex.unlock();
  Queue();
}

void AsyncContextReleaser::Execute(Napi::Env env) {
  _contexts_mutex.lock();
  while (!_contexts.empty()) {
    Napi::HandleScope scope(env);
    auto context = _contexts.front();
    delete context;
    _contexts.pop();
  }
  _contexts_mutex.unlock();
}

AsyncContextReleaser* AsyncContextReleaser::GetDefault() {
  return _default;
}

void AsyncContextReleaser::Init(Napi::Env env, Napi::Object) {
  auto func = DefineClass(env, "AsyncContextReleaser", {});
  constructor() = Napi::Persistent(func);
  constructor().SuppressDestruct();
  auto object = constructor().New({});
  _default = Unwrap(object);
  _default->Ref();
}

}  // namespace node_webrtc
