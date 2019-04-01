#include "src/dictionaries/webrtc/rtc_configuration.h"

#include "src/converters.h"
#include "src/converters/object.h"
#include "src/converters/v8.h"
#include "src/dictionaries/webrtc/ice_server.h"
#include "src/enums/webrtc/bundle_policy.h"
#include "src/enums/webrtc/ice_transports_type.h"
#include "src/enums/webrtc/rtcp_mux_policy.h"
#include "src/enums/webrtc/sdp_semantics.h"
#include "src/functional/curry.h"
#include "src/functional/operators.h"

namespace node_webrtc {

FROM_JS_IMPL(webrtc::PeerConnectionInterface::RTCConfiguration, value) {
  // NOTE(mroberts): Allow overriding the default SdpSemantics via environment variable.
  // Makes web-platform-tests easier to run.
  auto sdp_semantics_env = std::getenv("SDP_SEMANTICS");
  auto sdp_semantics_str = sdp_semantics_env ? std::string(sdp_semantics_env) : std::string();
  auto sdp_semantics = From<webrtc::SdpSemantics>(sdp_semantics_str).FromValidation(webrtc::SdpSemantics::kPlanB);
  return From<v8::Local<v8::Object>>(value).FlatMap<webrtc::PeerConnectionInterface::RTCConfiguration>([sdp_semantics](auto object) {
    return curry(CreateRTCConfiguration)
        % GetOptional<std::vector<webrtc::PeerConnectionInterface::IceServer>>(object, "iceServers", std::vector<webrtc::PeerConnectionInterface::IceServer>())
        * GetOptional<webrtc::PeerConnectionInterface::IceTransportsType>(object, "iceTransportPolicy", webrtc::PeerConnectionInterface::IceTransportsType::kAll)
        * GetOptional<webrtc::PeerConnectionInterface::BundlePolicy>(object, "bundlePolicy", webrtc::PeerConnectionInterface::BundlePolicy::kBundlePolicyBalanced)
        * GetOptional<webrtc::PeerConnectionInterface::RtcpMuxPolicy>(object, "rtcpMuxPolicy", webrtc::PeerConnectionInterface::RtcpMuxPolicy::kRtcpMuxPolicyRequire)
        * GetOptional<std::string>(object, "peerIdentity")
        * GetOptional<std::vector<v8::Local<v8::Object>>>(object, "certificates")
        // TODO(mroberts): Implement EnforceRange and change to uint8_t.
        * GetOptional<uint8_t>(object, "iceCandidatePoolSize", 0)
        * GetOptional<webrtc::SdpSemantics>(object, "sdpSemantics", sdp_semantics);
  });
}

}  // namespace node_webrtc
