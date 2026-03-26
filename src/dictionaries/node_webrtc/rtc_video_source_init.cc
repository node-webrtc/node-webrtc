#include "src/dictionaries/node_webrtc/rtc_video_source_init.hh"

#include "src/functional/maybe.hh"
#include "src/functional/validation.hh"

namespace node_webrtc {

#define RTC_VIDEO_SOURCE_INIT_FN CreateRTCVideoSourceInit

static Validation<RTC_VIDEO_SOURCE_INIT>
RTC_VIDEO_SOURCE_INIT_FN(const bool isScreencast,
                         const Maybe<bool> needsDenoising) {
  return Pure<RTC_VIDEO_SOURCE_INIT>({isScreencast, needsDenoising});
}

} // namespace node_webrtc

#define DICT(X) RTC_VIDEO_SOURCE_INIT##X
#include "src/dictionaries/macros/impls.hh"
#undef DICT
