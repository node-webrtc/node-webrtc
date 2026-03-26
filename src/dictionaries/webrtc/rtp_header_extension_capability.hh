#pragma once

#include "src/converters/napi.hh"

namespace webrtc {
struct RtpHeaderExtensionCapability;
}

namespace node_webrtc {

DECLARE_TO_NAPI(webrtc::RtpHeaderExtensionCapability)

} // namespace node_webrtc
