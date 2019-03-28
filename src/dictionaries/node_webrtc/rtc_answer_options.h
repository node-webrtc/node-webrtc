#pragma once

#include <webrtc/api/peer_connection_interface.h>

// IWYU pragma: no_include "src/dictionaries/macros/impls.h"

namespace node_webrtc {

struct RTCAnswerOptions {
  RTCAnswerOptions(): options(webrtc::PeerConnectionInterface::RTCOfferAnswerOptions()) {}
  explicit RTCAnswerOptions(const webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options): options(options) {}
  const webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
};

}  // namespace node_webrtc

#define RTC_ANSWER_OPTIONS RTCAnswerOptions
#define RTC_ANSWER_OPTIONS_NAME "RTCAnswerOptions"
#define RTC_ANSWER_OPTIONS_LIST \
  DEFAULT(bool, voiceActivityDetection, "voiceActivityDetection", true)

#define DICT(X) RTC_ANSWER_OPTIONS ## X
#include "src/dictionaries/macros/decls.h"
#undef DICT
