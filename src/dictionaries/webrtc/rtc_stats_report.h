#pragma once

#include "src/converters/v8.h"

namespace rtc { template <typename T> class scoped_refptr; }
namespace webrtc { class RTCStatsReport; }

namespace node_webrtc {

DECLARE_TO_JS(rtc::scoped_refptr<webrtc::RTCStatsReport>)

}  // namespace node_webrtc
