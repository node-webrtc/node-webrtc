#include "src/dictionaries/webrtc/rtc_stats.h"

#include <iosfwd>
#include <string>
#include <utility>
#include <vector>

#include <node-addon-api/napi.h>
#include <webrtc/api/stats/rtc_stats.h>

#include "src/dictionaries/macros/napi.h"
#include "src/dictionaries/webrtc/rtc_stats_member_interface.h"  // IWYU pragma: keep
#include "src/functional/validation.h"

namespace node_webrtc {

TO_NAPI_IMPL(const webrtc::RTCStats*, pair) {
  auto env = pair.first;
  Napi::EscapableHandleScope scope(env);
  auto value = pair.second;
  NODE_WEBRTC_CREATE_OBJECT_OR_RETURN(env, stats)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, stats, "id", value->id())
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, stats, "timestamp", value->timestamp_us() / 1000.0)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, stats, "type", std::string(value->type()))
  for (const webrtc::RTCStatsMemberInterface* member : value->Members()) {
    if (member->is_defined()) {
      NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, stats, member->name(), member)
    }
  }
  return Pure(scope.Escape(stats));
}

}  // namespace node_webrtc
