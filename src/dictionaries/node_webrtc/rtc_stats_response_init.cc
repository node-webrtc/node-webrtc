#include "src/dictionaries/node_webrtc/rtc_stats_response_init.h"

#include <node-addon-api/napi.h>

#include "src/functional/validation.h"
#include "src/interfaces/rtc_stats_response.h"

namespace node_webrtc {

TO_NAPI_IMPL(RTCStatsResponseInit, pair) {
  Napi::EscapableHandleScope scope(pair.first);
  return Pure(scope.Escape(RTCStatsResponse::Create(pair.second.first, pair.second.second)->Value()));
}

}  // namespace node_webrtc
