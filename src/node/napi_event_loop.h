#pragma once

#include <atomic>
#include <mutex>

#include "src/node/event_queue.h"
#include "src/node/events.h"

namespace node_webrtc {

namespace napi {

template <typename T>
class EventLoop: private EventQueue<T> {
 public:
  virtual ~EventLoop() = default;

  void Dispatch(std::unique_ptr<Event<T>> event) {
    this->Enqueue(std::move(event));
    _work_mutex.lock();
    if (_work && _run_mutex.try_lock()) {
      napi_queue_async_work(_env, _work);
    }
    _work_mutex.unlock();
  }

  bool should_stop() const {
    return _should_stop;
  }

 protected:
  EventLoop(Napi::Env env, const char* name, T& target): _env(env), _target(target) {
    napi_value resource_id;
    napi_status status = napi_create_string_latin1(
            _env,
            name,
            NAPI_AUTO_LENGTH,
            &resource_id);
    {
      using Napi::Error;
      NAPI_THROW_IF_FAILED_VOID(_env, status)
    }
    status = napi_create_async_work(
            _env,
            nullptr,
            resource_id,
            DoNothing,
            CallRun,
            this,
            &_work);
    {
      using Napi::Error;
      NAPI_THROW_IF_FAILED_VOID(_env, status)
    }
  }

  virtual void DidStop() {
    // Do nothing.
  }

  virtual void Run() {
    if (!_should_stop) {
      while (auto event = this->Dequeue()) {
        event->Dispatch(_target);
        if (_should_stop) {
          break;
        }
      }
    }
    if (_should_stop) {
      _work_mutex.lock();
      napi_delete_async_work(_env, _work);
      _work = nullptr;
      _work_mutex.unlock();
      DidStop();
    }
    _run_mutex.unlock();
  }

  virtual void Stop() {
    _should_stop = true;
    Dispatch(Event<T>::Create());
  }

 private:
  static void DoNothing(napi_env env, void* self) {
    (void) env;
    (void) self;
    // Do nothing.
  }

  static void CallRun(napi_env env, napi_status status, void* self) {
    (void) env;
    (void) status;
    static_cast<EventLoop<T>*>(self)->Run();
  }

  napi_env _env;
  std::mutex _run_mutex{};
  std::atomic<bool> _should_stop = {false};
  T& _target;
  napi_async_work _work;
  std::mutex _work_mutex{};
};

}  // namespace napi

}  // namespace node_webrtc
