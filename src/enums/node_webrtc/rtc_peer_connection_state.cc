#include "src/enums/node_webrtc/rtc_peer_connection_state.h"

#include "src/functional/validation.h"

namespace node_webrtc {

CONVERTER_IMPL(webrtc::PeerConnectionInterface::IceConnectionState, RTCPeerConnectionState, state) {
  switch (state) {
    case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionNew:
      return Pure(kNew);
    case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionChecking:
      return Pure(kConnecting);
    case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionConnected:
    case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionCompleted:
      return Pure(kConnected);
    case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionDisconnected:
      return Pure(kDisconnected);
    case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionFailed:
      return Pure(kFailed);
    case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionClosed:
      return Pure(kClosed);
    case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionMax:
      return Validation<node_webrtc::RTCPeerConnectionState>::Invalid(
              "WebRTC\'s RTCPeerConnection has an ICE connection state \"max\", but I have no idea"
              "what this means. If you see this error, file a bug on https://github.com/js-platform/node-webrtc");
  }
}
}  // namespace node_webrtc

#define ENUM(X) RTC_PEER_CONNECTION_STATE ## X
#include "src/enums/macros/impls.h"
#undef ENUM
