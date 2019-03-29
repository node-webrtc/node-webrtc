#include "src/dictionaries/node_webrtc/rtc_session_description_init.h"

#include "src/functional/validation.h"

namespace node_webrtc {

#define RTC_SESSION_DESCRIPTION_INIT_FN CreateValidRTCSessionDescriptionInit

static Validation<RTC_SESSION_DESCRIPTION_INIT> RTC_SESSION_DESCRIPTION_INIT_FN(
    const RTCSdpType type,
    const std::string& sdp) {
  return Pure(CreateRTCSessionDescriptionInit(type, sdp));
}

}  // namespace node_webrtc

#define DICT(X) RTC_SESSION_DESCRIPTION_INIT ## X
#include "src/dictionaries/macros/impls.h"
#undef DICT
