#pragma once

#include "src/converters.h"

namespace webrtc {

struct RtpExtension;

typedef RtpExtension RtpHeaderExtensionParameters;

}  // namespace webrtc

namespace node_webrtc {

DECLARE_TO_JS(webrtc::RtpHeaderExtensionParameters)

}  // namespace node_webrtc
