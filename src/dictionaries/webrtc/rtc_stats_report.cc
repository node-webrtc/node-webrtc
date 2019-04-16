#include "src/dictionaries/webrtc/rtc_stats_report.h"

#include <node-addon-api/napi.h>
#include <webrtc/api/scoped_refptr.h>  // IWYU pragma: keep
#include <webrtc/api/stats/rtc_stats_report.h>  // IWYU pragma: keep

#include "src/dictionaries/webrtc/rtc_stats.h"  // IWYU pragma: keep
#include "src/functional/validation.h"

namespace node_webrtc {

TO_NAPI_IMPL(rtc::scoped_refptr<webrtc::RTCStatsReport>, pair) {
  (void) pair;
  return Validation<Napi::Value>::Invalid("// FIXME(mroberts): How de we create Maps?");
}

}  // namespace node_webrtc
