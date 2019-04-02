#include "src/dictionaries/node_webrtc/rtc_stats_response_init.h"

#include <nan.h>
#include <node-addon-api/napi.h>
#include <v8.h>

#include "src/functional/validation.h"
#include "src/interfaces/rtc_stats_response.h"

namespace node_webrtc {

TO_JS_IMPL(RTCStatsResponseInit, init) {
  Nan::EscapableHandleScope scope;
  return Pure(scope.Escape(RTCStatsResponse::Create(init.first, init.second)->handle().As<v8::Value>()));
}

TO_NAPI_IMPL(RTCStatsResponseInit, pair) {
  (void) pair;
  return Validation<Napi::Value>::Invalid("// FIXME(mroberts): Implement me.");
}

}  // namespace node_webrtc
