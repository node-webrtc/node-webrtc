#pragma once

#include <iosfwd>
#include <string>

#include "src/enums/node_webrtc/rtc_sdp_type.h"

// IWYU pragma: no_forward_declare node_webrtc::RTCSessionDescriptionInit
// IWYU pragma: no_include "src/dictionaries/macros/impls.h"

#define RTC_SESSION_DESCRIPTION_INIT RTCSessionDescriptionInit
#define RTC_SESSION_DESCRIPTION_INIT_LIST \
  REQUIRED(RTCSdpType, type, "type") \
  DEFAULT(std::string, sdp, "sdp", "")

#define DICT(X) RTC_SESSION_DESCRIPTION_INIT ## X
#include "src/dictionaries/macros/def.h"
#include "src/dictionaries/macros/decls.h"
#undef DICT

namespace node_webrtc {

static inline RTC_SESSION_DESCRIPTION_INIT CreateRTCSessionDescriptionInit(
    const RTCSdpType type,
    const std::string& sdp) {
  return {type, sdp};
}

}  // namespace node_webrtc
