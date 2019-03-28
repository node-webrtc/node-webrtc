#include "src/dictionaries/node_webrtc/rtc_offer_options.h"

#include "src/functional/maybe.h"

// IWYU pragma: no_include <api/peer_connection_interface.h>

namespace node_webrtc {

#define RTC_OFFER_OPTIONS_FN CreateRTCOfferOptions

static RTC_OFFER_OPTIONS RTC_OFFER_OPTIONS_FN(
    const bool voiceActivityDetection,
    const bool iceRestart,
    const Maybe<bool> offerToReceiveAudio,
    const Maybe<bool> offerToReceiveVideo) {
  webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
  options.ice_restart = iceRestart;
  options.voice_activity_detection = voiceActivityDetection;
  options.offer_to_receive_audio = offerToReceiveAudio.Map([](auto boolean) {
    return boolean
        ? webrtc::PeerConnectionInterface::RTCOfferAnswerOptions::kOfferToReceiveMediaTrue
        : 0;
  }).FromMaybe(webrtc::PeerConnectionInterface::RTCOfferAnswerOptions::kUndefined);
  options.offer_to_receive_video = offerToReceiveVideo.Map([](auto boolean) {
    return boolean
        ? webrtc::PeerConnectionInterface::RTCOfferAnswerOptions::kOfferToReceiveMediaTrue
        : 0;
  }).FromMaybe(webrtc::PeerConnectionInterface::RTCOfferAnswerOptions::kUndefined);
  return RTC_OFFER_OPTIONS(options);
}

}  // namespace node_webrtc

#define DICT(X) RTC_OFFER_OPTIONS ## X
#include "src/dictionaries/macros/impls.h"
#undef DICT
