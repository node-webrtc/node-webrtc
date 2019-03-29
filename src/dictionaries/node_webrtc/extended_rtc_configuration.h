#pragma once

#include <webrtc/api/peer_connection_interface.h>

#include "src/dictionaries/node_webrtc/unsigned_short_range.h"

namespace node_webrtc {

struct ExtendedRTCConfiguration {
  ExtendedRTCConfiguration():
    configuration(webrtc::PeerConnectionInterface::RTCConfiguration()),
    portRange(UnsignedShortRange()) {}

  ExtendedRTCConfiguration(
      const webrtc::PeerConnectionInterface::RTCConfiguration& configuration,
      const UnsignedShortRange portRange):
    configuration(configuration),
    portRange(portRange) {}

  webrtc::PeerConnectionInterface::RTCConfiguration configuration;
  UnsignedShortRange portRange;
};

DECLARE_TO_AND_FROM_JS(ExtendedRTCConfiguration)

}  // namespace node_webrtc
