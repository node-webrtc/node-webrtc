#include "src/dictionaries/webrtc/ice_candidate_interface.h"

#include <webrtc/api/jsep.h>

#include "src/dictionaries/macros/napi.h"
#include "src/functional/maybe.h"
#include "src/functional/validation.h"

namespace node_webrtc {

#define ICE_CANDIDATE_INTERFACE_FN CreateIceCandidateInterface
#define ICE_CANDIDATE_INTERFACE_LIST \
  DICT_DEFAULT(std::string, candidate, "candidate", "") \
  DICT_DEFAULT(std::string, sdpMid, "sdpMid", "") \
  DICT_DEFAULT(int, sdpMLineIndex, "sdpMLineIndex", 0) \
  DICT_OPTIONAL(std::string, usernameFragment, "usernameFragment")

static Validation<webrtc::IceCandidateInterface*> ICE_CANDIDATE_INTERFACE_FN(
    const std::string& candidate,
    const std::string& sdpMid,
    const int sdpMLineIndex,
    const Maybe<std::string>&) {
  webrtc::SdpParseError error;
  auto candidate_ = webrtc::CreateIceCandidate(sdpMid, sdpMLineIndex, candidate, &error);
  if (!candidate_) {
    return Validation<webrtc::IceCandidateInterface*>::Invalid(error.description);
  }
  return Pure(candidate_);
}

TO_JS_IMPL(webrtc::IceCandidateInterface*, value) {
  Nan::EscapableHandleScope scope;

  if (!value) {
    return Validation<v8::Local<v8::Value>>::Invalid("RTCIceCandidate is null");
  }

  std::string candidate;
  if (!value->ToString(&candidate)) {
    return Validation<v8::Local<v8::Value>>::Invalid("Failed to print the candidate string. This is pretty weird. File a bug on https://github.com/js-platform/node-webrtc");
  }

  auto object = Nan::New<v8::Object>();
  object->Set(Nan::New("candidate").ToLocalChecked(), Nan::New(candidate).ToLocalChecked());
  object->Set(Nan::New("sdpMid").ToLocalChecked(), Nan::New(value->sdp_mid()).ToLocalChecked());
  object->Set(Nan::New("sdpMLineIndex").ToLocalChecked(), Nan::New(value->sdp_mline_index()));

  return Pure(scope.Escape(object.As<v8::Value>()));
}

CONVERT_VIA(v8::Local<v8::Value>, webrtc::IceCandidateInterface*, std::shared_ptr<webrtc::IceCandidateInterface>)

FROM_NAPI_IMPL(std::shared_ptr<webrtc::IceCandidateInterface>, napi_value) {
  return From<std::shared_ptr<webrtc::IceCandidateInterface>>(napi::UnsafeToV8(napi_value));
}

TO_NAPI_IMPL(webrtc::IceCandidateInterface*, pair) {
  auto env = pair.first;
  Napi::EscapableHandleScope scope(env);

  auto value = pair.second;
  if (!value) {
    return Validation<Napi::Value>::Invalid("RTCIceCandidate is null");
  }

  std::string candidate;
  if (!value->ToString(&candidate)) {
    return Validation<Napi::Value>::Invalid("Failed to print the candidate string. This is pretty weird. File a bug on https://github.com/js-platform/node-webrtc");
  }

  NODE_WEBRTC_CREATE_OBJECT_OR_RETURN(env, object)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "candidate", candidate)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "sdpMid", value->sdp_mid())
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "sdpMLineIndex", value->sdp_mline_index())

  return Pure(scope.Escape(object));
}

}  // namespace node_webrtc

#define DICT(X) ICE_CANDIDATE_INTERFACE ## X
#include "src/dictionaries/macros/impls.h"
#undef DICT
