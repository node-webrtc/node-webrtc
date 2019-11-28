#include "src/converters/webrtc.h"

#include "src/functional/validation.h"

namespace node_webrtc {

TO_NAPI_IMPL(rtc::Buffer*, pair) {
  auto env = pair.first;
  Napi::EscapableHandleScope scope(env);
  auto buffer = pair.second;
  auto size = buffer->size();
  auto data = new uint8_t[size];
  memcpy(reinterpret_cast<void*>(data), reinterpret_cast<const void*>(buffer->data()), size);
  auto maybeArrayBuffer = Napi::ArrayBuffer::New(env, data, size, [](Napi::Env, void* data) {
    delete[] static_cast<uint8_t*>(data);
  });
  if (maybeArrayBuffer.Env().IsExceptionPending()) {
    return Validation<Napi::Value>::Invalid(maybeArrayBuffer.Env().GetAndClearPendingException().Message());
  }
  return Pure<Napi::Value>(scope.Escape(maybeArrayBuffer));
}

}  // namespace node_webrtc
