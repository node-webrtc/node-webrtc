#include "src/dictionaries/webrtc/video_frame.h"

#include <utility>

#include <nan.h>
#include <node-addon-api/napi.h>
#include <v8.h>
#include <webrtc/api/scoped_refptr.h>
#include <webrtc/api/video/video_frame.h>

#include "src/converters.h"
#include "src/dictionaries/macros/napi.h"
#include "src/dictionaries/webrtc/video_frame_buffer.h"  // IWYU pragma: keep
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

TO_NAPI_IMPL(webrtc::VideoFrame, pair) {
  auto env = pair.first;
  Napi::EscapableHandleScope scope(env);
  auto value = pair.second;
  NODE_WEBRTC_CREATE_OBJECT_OR_RETURN(env, frame)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, frame, "width", value.width())
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, frame, "height", value.height())
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, frame, "rotation", static_cast<int>(value.rotation()))
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, frame, "data", value.video_frame_buffer())
  return Pure(scope.Escape(frame));
}

}  // namespace node_webrtc
