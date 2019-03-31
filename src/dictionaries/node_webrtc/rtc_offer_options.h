#pragma once

#include <webrtc/api/peer_connection_interface.h>

// IWYU pragma: no_include "src/dictionaries/macros/impls.h"

namespace node_webrtc {

struct RTCOfferOptions {
  RTCOfferOptions(): options(webrtc::PeerConnectionInterface::RTCOfferAnswerOptions()) {}
  explicit RTCOfferOptions(const webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options): options(options) {}
  const webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
};

}  // namespace node_webrtc

#define RTC_OFFER_OPTIONS RTCOfferOptions
#define RTC_OFFER_OPTIONS_LIST \
  DICT_DEFAULT(bool, voiceActivityDetection, "voiceActivityDetection", true) \
  DICT_DEFAULT(bool, iceRestart, "iceRestart", false) \
  DICT_OPTIONAL(bool, offerToReceiveAudio, "offerToReceiveAudio") \
  DICT_OPTIONAL(bool, offerToReceiveVideo, "offerToReceiveVideo")

#define DICT(X) RTC_OFFER_OPTIONS ## X
#include "src/dictionaries/macros/decls.h"
#undef DICT
