#include "src/dictionaries/node_webrtc/rtc_answer_options.h"

#include "src/functional/validation.h"

namespace node_webrtc {

#define RTC_ANSWER_OPTIONS_FN CreateRTCAnswerOptions

static Validation<RTC_ANSWER_OPTIONS> RTC_ANSWER_OPTIONS_FN(const bool voiceActivityDetection) {
  webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
  options.voice_activity_detection = voiceActivityDetection;
  return Pure(RTC_ANSWER_OPTIONS(options));
}

}  // namespace node_webrtc

#define DICT(X) RTC_ANSWER_OPTIONS ## X
#include "src/dictionaries/macros/impls.h"
#undef DICT
