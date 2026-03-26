#if defined(_WIN32)
// Needed for std::getenv on Windows
#define _CRT_SECURE_NO_WARNINGS // NOLINT(bugprone-reserved-identifier)
#endif

#include "src/dictionaries/webrtc/rtc_configuration.hh"

#include "src/converters.hh"
#include "src/converters/object.hh"
#include "src/dictionaries/webrtc/ice_server.hh"
#include "src/enums/webrtc/bundle_policy.hh"
#include "src/enums/webrtc/ice_transports_type.hh"
#include "src/enums/webrtc/rtcp_mux_policy.hh"
#include "src/enums/webrtc/sdp_semantics.hh"
#include "src/functional/curry.hh"
#include "src/functional/operators.hh"

namespace node_webrtc {

FROM_NAPI_IMPL(webrtc::PeerConnectionInterface::RTCConfiguration, value) {
  // NOTE(mroberts): Allow overriding the default SdpSemantics via environment
  // variable. Makes web-platform-tests easier to run.
  auto sdp_semantics_env = std::getenv("SDP_SEMANTICS");
  auto sdp_semantics_str =
      sdp_semantics_env ? std::string(sdp_semantics_env) : std::string();
  auto sdp_semantics = From<webrtc::SdpSemantics>(sdp_semantics_str)
                           .FromValidation(webrtc::SdpSemantics::kUnifiedPlan);
  return From<Napi::Object>(value)
      .FlatMap<
          webrtc::PeerConnectionInterface::RTCConfiguration>([sdp_semantics](
                                                                 auto object) {
        return curry(CreateRTCConfiguration) %
               GetOptional<
                   std::vector<webrtc::PeerConnectionInterface::IceServer>>(
                   object, "iceServers",
                   std::vector<webrtc::PeerConnectionInterface::IceServer>()) *
               GetOptional<webrtc::PeerConnectionInterface::IceTransportsType>(
                   object, "iceTransportPolicy",
                   webrtc::PeerConnectionInterface::IceTransportsType::kAll) *
               GetOptional<webrtc::PeerConnectionInterface::BundlePolicy>(
                   object, "bundlePolicy",
                   webrtc::PeerConnectionInterface::BundlePolicy::
                       kBundlePolicyBalanced) *
               GetOptional<webrtc::PeerConnectionInterface::RtcpMuxPolicy>(
                   object, "rtcpMuxPolicy",
                   webrtc::PeerConnectionInterface::RtcpMuxPolicy::
                       kRtcpMuxPolicyRequire) *
               GetOptional<std::string>(object, "peerIdentity") *
               GetOptional<std::vector<Napi::Object>>(object, "certificates")
               // TODO(mroberts): Implement EnforceRange and change to uint8_t.
               * GetOptional<uint8_t>(object, "iceCandidatePoolSize", 0) *
               GetOptional<webrtc::SdpSemantics>(object, "sdpSemantics",
                                                 sdp_semantics);
      });
}

} // namespace node_webrtc
