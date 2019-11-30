#pragma once

#include "src/converters/napi.h"

namespace webrtc { struct RtpExtension; }
namespace webrtc { typedef RtpExtension RtpHeaderExtensionParameters; }

namespace node_webrtc {

DECLARE_TO_AND_FROM_NAPI(webrtc::RtpHeaderExtensionParameters)

}  // namespace node_webrtc
