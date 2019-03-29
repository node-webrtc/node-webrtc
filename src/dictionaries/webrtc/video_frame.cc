#include "src/dictionaries/webrtc/video_frame.h"

#include <nan.h>
#include <webrtc/api/video/video_frame.h>
#include <v8.h>

#include "src/converters/v8.h"
#include "src/dictionaries/webrtc/video_frame_buffer.h"
#include "src/functional/validation.h"

namespace node_webrtc {

TO_JS_IMPL(webrtc::VideoFrame, value) {
  Nan::EscapableHandleScope scope;
  auto frame = Nan::New<v8::Object>();
  frame->Set(Nan::New("width").ToLocalChecked(), From<v8::Local<v8::Value>>(value.width()).UnsafeFromValid());
  frame->Set(Nan::New("height").ToLocalChecked(), From<v8::Local<v8::Value>>(value.height()).UnsafeFromValid());
  frame->Set(Nan::New("rotation").ToLocalChecked(), From<v8::Local<v8::Value>>(static_cast<int>(value.rotation())).UnsafeFromValid());
  auto maybeData = From<v8::Local<v8::Value>>(value.video_frame_buffer());
  if (maybeData.IsInvalid()) {
    return Validation<v8::Local<v8::Value>>::Invalid(maybeData.ToErrors()[0]);
  }
  auto data = maybeData.UnsafeFromValid();
  frame->Set(Nan::New("data").ToLocalChecked(), data);
  return Pure(scope.Escape(frame).As<v8::Value>());
}

}  // namespace node_webrtc
