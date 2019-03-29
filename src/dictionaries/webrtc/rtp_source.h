#pragma once

#include "src/converters.h"

namespace webrtc { class RtpSource; }

namespace node_webrtc {

DECLARE_TO_JS(webrtc::RtpSource)

}  // namespace node_webrtc
