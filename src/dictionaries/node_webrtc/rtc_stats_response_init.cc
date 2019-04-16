#include "src/dictionaries/node_webrtc/rtc_stats_response_init.h"

#include <node-addon-api/napi.h>

#include "src/functional/validation.h"

namespace node_webrtc {

TO_NAPI_IMPL(RTCStatsResponseInit, pair) {
  (void) pair;
  return Validation<Napi::Value>::Invalid("// FIXME(mroberts): Implement me.");
}

}  // namespace node_webrtc
