#include "src/dictionaries/node_webrtc/rtc_session_description_init.h"

#include <webrtc/api/jsep.h>

#include "src/functional/curry.h"
#include "src/functional/operators.h"
#include "src/functional/validation.h"

namespace node_webrtc {

#define RTC_SESSION_DESCRIPTION_INIT_FN CreateValidRTCSessionDescriptionInit

static Validation<RTC_SESSION_DESCRIPTION_INIT> RTC_SESSION_DESCRIPTION_INIT_FN(
    const RTCSdpType type,
    const std::string& sdp) {
  return Pure(CreateRTCSessionDescriptionInit(type, sdp));
}

TO_JS_IMPL(RTCSessionDescriptionInit, init) {
  Nan::EscapableHandleScope scope;
  auto maybeType = From<std::string>(init.type);
  if (maybeType.IsInvalid()) {
    return Validation<v8::Local<v8::Value>>::Invalid(maybeType.ToErrors()[0]);
  }
  auto object = Nan::New<v8::Object>();
  object->Set(Nan::New("sdp").ToLocalChecked(), Nan::New(init.sdp).ToLocalChecked());
  object->Set(Nan::New("type").ToLocalChecked(), Nan::New(maybeType.UnsafeFromValid()).ToLocalChecked());
  return Pure(scope.Escape(object.As<v8::Value>()));
}

CONVERTER_IMPL(RTCSessionDescriptionInit, webrtc::SessionDescriptionInterface*, init) {
  std::string type_;
  switch (init.type) {
    case RTCSdpType::kOffer:
      type_ = "offer";
      break;
    case RTCSdpType::kPrAnswer:
      type_ = "pranswer";
      break;
    case RTCSdpType::kAnswer:
      type_ = "answer";
      break;
    case RTCSdpType::kRollback:
      return Validation<webrtc::SessionDescriptionInterface*>::Invalid("Rollback is not currently supported");
  }
  webrtc::SdpParseError error;
  auto description = webrtc::CreateSessionDescription(type_, init.sdp, &error);
  if (!description) {
    return Validation<webrtc::SessionDescriptionInterface*>::Invalid(error.description);
  }
  return Pure(description);
}

CONVERTER_IMPL(webrtc::SessionDescriptionInterface*, RTCSessionDescriptionInit, description) {
  if (!description) {
    return Validation<RTCSessionDescriptionInit>::Invalid("RTCSessionDescription is null");
  }
  std::string sdp;
  if (!description->ToString(&sdp)) {
    return Validation<RTCSessionDescriptionInit>::Invalid(
            "Failed to print the SDP. This is pretty weird. File a bug on https://github.com/js-platform/node-webrtc");
  }
  return curry(CreateRTCSessionDescriptionInit)
      % From<RTCSdpType>(description->type())
      * Validation<std::string>(sdp);
}

FROM_JS_IMPL(webrtc::SessionDescriptionInterface*, value) {
  return From<RTCSessionDescriptionInit>(value)
      .FlatMap<webrtc::SessionDescriptionInterface*>(Converter<RTCSessionDescriptionInit, webrtc::SessionDescriptionInterface*>::Convert);
}

TO_JS_IMPL(const webrtc::SessionDescriptionInterface*, value) {
  Nan::EscapableHandleScope scope;

  if (!value) {
    return node_webrtc::Validation<v8::Local<v8::Value>>::Invalid("RTCSessionDescription is null");
  }

  std::string sdp;
  if (!value->ToString(&sdp)) {
    return node_webrtc::Validation<v8::Local<v8::Value>>::Invalid("Failed to print the SDP. This is pretty weird. File a bug on https://github.com/js-platform/node-webrtc");
  }

  auto object = Nan::New<v8::Object>();
  object->Set(Nan::New("sdp").ToLocalChecked(), Nan::New(sdp).ToLocalChecked());
  object->Set(Nan::New("type").ToLocalChecked(), Nan::New(value->type()).ToLocalChecked());

  return node_webrtc::Pure(scope.Escape(object.As<v8::Value>()));
}
}  // namespace node_webrtc

#define DICT(X) RTC_SESSION_DESCRIPTION_INIT ## X
#include "src/dictionaries/macros/impls.h"
#undef DICT
