#pragma once

#include <mutex>

#include <node_api.h>
#include <node-addon-api/napi.h>

namespace node_webrtc {

class Deferrer {
 public:
  explicit Deferrer(Napi::Env);
  virtual ~Deferrer();

 protected:
  virtual void Execute(Napi::Env) = 0;
  void Queue();

 private:
  static void DoNothing(napi_env, void*);
  static void CallExecute(napi_env, napi_status, void*);
  Napi::Env _env;
  napi_async_work _work = nullptr;
  std::mutex _work_mutex{};
};

}  // namespace node_webrtc
