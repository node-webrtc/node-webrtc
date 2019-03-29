#include "src/dictionaries/webrtc/rtp_source.h"

#include <webrtc/api/rtp_receiver_interface.h>

#include <nan.h>
#include <v8.h>

#include "src/functional/validation.h"

TO_JS_IMPL(webrtc::RtpSource, source) {
  Nan::EscapableHandleScope scope;
  auto object = Nan::New<v8::Object>();
  object->Set(Nan::New("timestamp").ToLocalChecked(), Nan::New<v8::Number>(source.timestamp_ms()));
  object->Set(Nan::New("source").ToLocalChecked(), Nan::New(source.source_id()));
  return Pure(scope.Escape(object).As<v8::Value>());
}

