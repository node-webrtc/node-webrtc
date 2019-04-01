#include "src/dictionaries/node_webrtc/extended_rtc_configuration.h"

#include "src/converters/object.h"
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

FROM_JS_IMPL(ExtendedRTCConfiguration, value) {
  return From<v8::Local<v8::Object>>(value).FlatMap<ExtendedRTCConfiguration>([](auto object) {
    return curry(CreateExtendedRTCConfiguration)
        % From<webrtc::PeerConnectionInterface::RTCConfiguration>(static_cast<v8::Local<v8::Value>>(object))
        * GetOptional<UnsignedShortRange>(object, "portRange", UnsignedShortRange());
  });
}

static v8::Local<v8::Value> ExtendedRTCConfigurationToJavaScript(
    const v8::Local<v8::Value> iceServers,
    const v8::Local<v8::Value> iceTransportPolicy,
    const v8::Local<v8::Value> bundlePolicy,
    const v8::Local<v8::Value> rtcpMuxPolicy,
    const v8::Local<v8::Value> iceCandidatePoolSize,
    const v8::Local<v8::Value> portRange,
    const v8::Local<v8::Value> sdpSemantics) {
  Nan::EscapableHandleScope scope;
  auto object = Nan::New<v8::Object>();
  object->Set(Nan::New("iceServers").ToLocalChecked(), iceServers);
  object->Set(Nan::New("iceTransportPolicy").ToLocalChecked(), iceTransportPolicy);
  object->Set(Nan::New("bundlePolicy").ToLocalChecked(), bundlePolicy);
  object->Set(Nan::New("rtcpMuxPolicy").ToLocalChecked(), rtcpMuxPolicy);
  object->Set(Nan::New("iceCandidatePoolSize").ToLocalChecked(), iceCandidatePoolSize);
  object->Set(Nan::New("portRange").ToLocalChecked(), portRange);
  object->Set(Nan::New("sdpSemantics").ToLocalChecked(), sdpSemantics);
  return scope.Escape(object);
}

TO_JS_IMPL(ExtendedRTCConfiguration, configuration) {
  return curry(ExtendedRTCConfigurationToJavaScript)
      % From<v8::Local<v8::Value>>(configuration.configuration.servers)
      * From<v8::Local<v8::Value>>(configuration.configuration.type)
      * From<v8::Local<v8::Value>>(configuration.configuration.bundle_policy)
      * From<v8::Local<v8::Value>>(configuration.configuration.rtcp_mux_policy)
      * Pure(Nan::New(configuration.configuration.ice_candidate_pool_size))
      * From<v8::Local<v8::Value>>(configuration.portRange)
      * From<v8::Local<v8::Value>>(configuration.configuration.sdp_semantics);
}


}  // namespace node_webrtc
