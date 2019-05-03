#include "src/dictionaries/webrtc/ice_candidate_interface.h"

#include <iosfwd>
#include <utility>

#include <node-addon-api/napi.h>
#include <webrtc/api/candidate.h>
#include <webrtc/api/jsep.h>
#include <webrtc/p2p/base/port.h>

#include "src/converters.h"
#include "src/dictionaries/macros/napi.h"
#include "src/enums/node_webrtc/rtc_ice_candidate_type.h"
#include "src/enums/node_webrtc/rtc_ice_component.h"
#include "src/functional/maybe.h"  // IWYU pragma: keep
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

FROM_NAPI_IMPL(std::shared_ptr<webrtc::IceCandidateInterface>, napi_value) {
  return From<webrtc::IceCandidateInterface*>(napi_value).Map([](auto candidate) {
    return std::shared_ptr<webrtc::IceCandidateInterface>(candidate);
  });
}

TO_NAPI_IMPL(webrtc::IceCandidateInterface*, pair) {
  auto env = pair.first;
  Napi::EscapableHandleScope scope(env);

  auto value = pair.second;
  if (!value) {
    return Validation<Napi::Value>::Invalid("RTCIceCandidate is null");
  }

  std::string candidate_string;
  if (!value->ToString(&candidate_string)) {
    return Validation<Napi::Value>::Invalid("Failed to print the candidate string. This is pretty weird. File a bug on https://github.com/node-webrtc/node-webrtc");
  }

  auto candidate = value->candidate();
  auto component = candidate.component() == 1 ? RTCIceComponent::kRtp : RTCIceComponent::kRtcp;

  const auto& candidate_type = candidate.type();
  auto type = RTCIceCandidateType::kHost;
  if (candidate_type == cricket::LOCAL_PORT_TYPE) {
    type = RTCIceCandidateType::kHost;
  } else if (candidate_type == cricket::STUN_PORT_TYPE) {
    type = RTCIceCandidateType::kSrflx;
  } else if (candidate_type == cricket::RELAY_PORT_TYPE) {
    type = RTCIceCandidateType::kRelay;
  } else if (candidate_type == cricket::PRFLX_PORT_TYPE) {
    type = RTCIceCandidateType::kPrflx;
  }

  NODE_WEBRTC_CREATE_OBJECT_OR_RETURN(env, object)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "candidate", candidate_string)

  const auto& mid = value->sdp_mid();
  if (mid.empty()) {
    object.Set("sdpMid", env.Null());
  } else {
    NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "sdpMid", mid)
  }

  auto mLineIndex = value->sdp_mline_index();
  if (mLineIndex < 0) {
    object.Set("sdpMLineIndex", env.Null());
  } else {
    NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "sdpMLineIndex", mLineIndex)
  }

  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "foundation", candidate.foundation())
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "component", component)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "priority", candidate.priority())
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "address", candidate.address().hostname())
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "protocol", candidate.protocol())
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "port", candidate.address().port())
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "type", type)

  const auto& tcpType = candidate.tcptype();
  if (tcpType.empty()) {
    object.Set("tcpType", env.Null());
  } else {
    NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "tcpType", candidate.tcptype())
  }

  if (type == RTCIceCandidateType::kHost) {
    object.Set("relatedAddress", env.Null());
    object.Set("relatedPort", env.Null());
  } else {
    NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "relatedAddress", candidate.related_address().hostname())
    NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "relatedPort", candidate.related_address().port())
  }

  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "usernameFragment", candidate.username())

  return Pure(scope.Escape(object));
}

}  // namespace node_webrtc

#define DICT(X) ICE_CANDIDATE_INTERFACE ## X
#include "src/dictionaries/macros/impls.h"
#undef DICT
