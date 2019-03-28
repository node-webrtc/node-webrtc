#include "src/dictionaries/node_webrtc/rtc_video_source_init.h"

#include "src/functional/maybe.h"

namespace node_webrtc {

#define RTC_VIDEO_SOURCE_INIT_FN CreateRTCVideoSourceInit

static RTC_VIDEO_SOURCE_INIT RTC_VIDEO_SOURCE_INIT_FN(
    const bool isScreencast,
    const Maybe<bool> needsDenoising) {
  return {isScreencast, needsDenoising};
}

}  // namespace node_webrtc

#define DICT(X) RTC_VIDEO_SOURCE_INIT ## X
#include "src/dictionaries/macros/impls.h"
#undef DICT
