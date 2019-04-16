#include "src/dictionaries/node_webrtc/extended_rtc_configuration.h"

#include "src/converters/object.h"
#include "src/dictionaries/macros/napi.h"
#include "src/dictionaries/webrtc/ice_server.h"
#include "src/dictionaries/webrtc/rtc_configuration.h"
#include "src/enums/webrtc/bundle_policy.h"
#include "src/enums/webrtc/ice_transports_type.h"
#include "src/enums/webrtc/rtcp_mux_policy.h"
#include "src/enums/webrtc/sdp_semantics.h"
#include "src/functional/curry.h"

namespace node_webrtc {

static ExtendedRTCConfiguration CreateExtendedRTCConfiguration(
    const webrtc::PeerConnectionInterface::RTCConfiguration& configuration,
    const UnsignedShortRange portRange) {
  return ExtendedRTCConfiguration(configuration, portRange);
}

FROM_NAPI_IMPL(ExtendedRTCConfiguration, value) {
  return From<Napi::Object>(value).FlatMap<ExtendedRTCConfiguration>([value](auto object) {
    return curry(CreateExtendedRTCConfiguration)
        % From<webrtc::PeerConnectionInterface::RTCConfiguration>(value)
        * napi::GetOptional<UnsignedShortRange>(object, "portRange", UnsignedShortRange());
  });
}

namespace napi {
static Validation<Napi::Value> ExtendedRTCConfigurationToJavaScript(
    const Napi::Value iceServers,
    const Napi::Value iceTransportPolicy,
    const Napi::Value bundlePolicy,
    const Napi::Value rtcpMuxPolicy,
    const Napi::Value iceCandidatePoolSize,
    const Napi::Value portRange,
    const Napi::Value sdpSemantics) {
  auto env = iceServers.Env();
  Napi::EscapableHandleScope scope(iceServers.Env());
  NODE_WEBRTC_CREATE_OBJECT_OR_RETURN(env, object)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "iceServers", iceServers)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "iceTransportPolicy", iceTransportPolicy)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "bundlePolicy", bundlePolicy)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "rtcpMuxPolicy", rtcpMuxPolicy)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "iceCandidatePoolSize", iceCandidatePoolSize)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "portRange", portRange)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "sdpSemantics", sdpSemantics)
  return Pure(scope.Escape(object));
}

}  // namespace napi

TO_NAPI_IMPL(ExtendedRTCConfiguration, pair) {
  return Validation<Napi::Value>::Join(curry(napi::ExtendedRTCConfigurationToJavaScript)
          % From<Napi::Value>(std::make_pair(pair.first, pair.second.configuration.servers))
          * From<Napi::Value>(std::make_pair(pair.first, pair.second.configuration.type))
          * From<Napi::Value>(std::make_pair(pair.first, pair.second.configuration.bundle_policy))
          * From<Napi::Value>(std::make_pair(pair.first, pair.second.configuration.rtcp_mux_policy))
          * From<Napi::Value>(std::make_pair(pair.first, pair.second.configuration.ice_candidate_pool_size))
          * From<Napi::Value>(std::make_pair(pair.first, pair.second.portRange))
          * From<Napi::Value>(std::make_pair(pair.first, pair.second.configuration.sdp_semantics)));
}

}  // namespace node_webrtc
