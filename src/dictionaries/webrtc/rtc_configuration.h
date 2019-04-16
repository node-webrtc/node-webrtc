#pragma once

#include <webrtc/api/peer_connection_interface.h>

#include "src/converters/napi.h"
#include "src/functional/maybe.h"

namespace node_webrtc {

namespace napi {

static inline webrtc::PeerConnectionInterface::RTCConfiguration CreateRTCConfiguration(
    const std::vector<webrtc::PeerConnectionInterface::IceServer>& iceServers,
    const webrtc::PeerConnectionInterface::IceTransportsType iceTransportsPolicy,
    const webrtc::PeerConnectionInterface::BundlePolicy bundlePolicy,
    const webrtc::PeerConnectionInterface::RtcpMuxPolicy rtcpMuxPolicy,
    const Maybe<std::string>&,
    const Maybe<std::vector<Napi::Object>>&,
    const uint32_t iceCandidatePoolSize,
    const webrtc::SdpSemantics sdpSemantics) {
  webrtc::PeerConnectionInterface::RTCConfiguration configuration;
  configuration.servers = iceServers;
  configuration.type = iceTransportsPolicy;
  configuration.bundle_policy = bundlePolicy;
  configuration.rtcp_mux_policy = rtcpMuxPolicy;
  configuration.ice_candidate_pool_size = iceCandidatePoolSize;
  configuration.sdp_semantics = sdpSemantics;
  return configuration;
}

}  // namespace napi

DECLARE_FROM_NAPI(webrtc::PeerConnectionInterface::RTCConfiguration)

}  // namespace node_webrtc
