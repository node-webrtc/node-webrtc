#pragma once

#include "src/converters/napi.h"

namespace webrtc { struct RtpHeaderExtensionCapability; }

namespace node_webrtc {

DECLARE_TO_NAPI(webrtc::RtpHeaderExtensionCapability)

}  // namespace node_webrtc
