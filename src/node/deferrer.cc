#include "src/node/deferrer.h"

#include <node_api.h>

namespace node_webrtc {

Deferrer::Deferrer(Napi::Env env): _env(env) {
  napi_create_async_work(
      _env,
      nullptr,
      Napi::String::New(env, "Deferrer"),
      &Deferrer::DoNothing,
      &Deferrer::CallExecute,
      this,
      &_work);
}

Deferrer::~Deferrer() {
  napi_delete_async_work(_env, _work);
  _work = nullptr;
}

void Deferrer::Queue() {
  if (_work_mutex.try_lock()) {
    Napi::HandleScope scope(_env);
    napi_queue_async_work(_env, _work);
  }
}

void Deferrer::DoNothing(napi_env, void*) {
  // Do nothing.
}

void Deferrer::CallExecute(napi_env, napi_status, void* data) {
  auto self = static_cast<Deferrer*>(data);
  self->_work_mutex.unlock();
  self->Execute(self->_env);
}

}  // namespace node_webrtc
