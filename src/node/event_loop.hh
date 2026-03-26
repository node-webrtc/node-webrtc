#pragma once

#include <atomic>
#include <mutex>

#include <node-addon-api/napi.h>
#include <uv.h>

#include "src/node/event_queue.hh"
#include "src/node/events.hh"

namespace node_webrtc {

template <typename T> class EventLoop : private EventQueue<T> {
public:
  EventLoop(const EventLoop &) = delete;
  EventLoop(EventLoop &&) = delete;
  EventLoop &operator=(const EventLoop &) = delete;
  EventLoop &operator=(EventLoop &&) = delete;
  virtual ~EventLoop() = default;

  void Dispatch(std::unique_ptr<Event<T>> event) {
    _lock.lock();
    if (!uv_is_closing(reinterpret_cast<uv_handle_t *>(&_async))) {
      this->Enqueue(std::move(event));
      uv_async_send(&_async);
    }
    _lock.unlock();
  }

  bool should_stop() const { return _should_stop; }

protected:
  EventLoop(Napi::Env env, Napi::AsyncContext *context, T &target)
      : _context(context), _env(env), _target(target) {
    uv_loop_t *loop{};
    auto status = napi_get_uv_event_loop(_env, &loop);
    {
      using Napi::Error;
      NAPI_THROW_IF_FAILED_VOID(_env, status)
    }

    uv_async_init(loop, &_async, [](auto handle) {
      auto self = static_cast<EventLoop<T> *>(handle->data);
      self->Run();
    });

    _async.data = this;
  }

  virtual void DidStop() {
    // Do nothing.
  }

  virtual void Run() {
    Napi::HandleScope scope(_env);
    if (!_should_stop) {
      while (auto event = this->Dequeue()) {
        Napi::CallbackScope callbackScope(_env, *_context);
        event->Dispatch(_target);
        if (_should_stop) {
          break;
        }
      }
    }
    if (_should_stop) {
      _lock.lock();
      uv_close(reinterpret_cast<uv_handle_t *>(&_async), [](auto handle) {
        auto self = static_cast<EventLoop<T> *>(handle->data);
        self->DidStop();
      });
      _lock.unlock();
    }
  }

  virtual void Stop() {
    _should_stop = true;
    Dispatch(Event<T>::Create());
  }

private:
  uv_async_t _async{};
  Napi::AsyncContext *_context;
  Napi::Env _env;
  std::mutex _lock;
  std::atomic<bool> _should_stop = {false};
  T &_target;
};

} // namespace node_webrtc
