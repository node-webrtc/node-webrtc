#include "src/dictionaries/node_webrtc/rtc_media_stream_init.hh"

#include "src/functional/validation.hh"

namespace node_webrtc {

#define RTC_MEDIA_STREAM_INIT_FN CreateRTCMediaStreamInit

static Validation<RTC_MEDIA_STREAM_INIT>
RTC_MEDIA_STREAM_INIT_FN(const std::string &id) {
  return Pure<RTC_MEDIA_STREAM_INIT>({id});
}

} // namespace node_webrtc

#define DICT(X) RTC_MEDIA_STREAM_INIT##X
#include "src/dictionaries/macros/impls.hh"
#undef DICT
