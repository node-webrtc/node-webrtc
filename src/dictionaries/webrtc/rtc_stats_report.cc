#include "src/dictionaries/webrtc/rtc_stats_report.h"

#include <nan.h>
#include <webrtc/api/scoped_refptr.h>
#include <webrtc/api/stats/rtc_stats_report.h>
#include <v8.h>

#include "src/dictionaries/webrtc/rtc_stats.h"  // IWYU pragma: keep
#include "src/functional/validation.h"

// IWYU pragma: no_include <api/stats/rtc_stats.h>

namespace node_webrtc {

TO_JS_IMPL(rtc::scoped_refptr<webrtc::RTCStatsReport>, value) {
  Nan::EscapableHandleScope scope;
  auto context = Nan::GetCurrentContext();
  auto report = v8::Map::New(context->GetIsolate());
  for (const webrtc::RTCStats& stats : *value) {
    report->Set(context, Nan::New(stats.id()).ToLocalChecked(), From<v8::Local<v8::Value>>(&stats).UnsafeFromValid()).IsEmpty();
  }
  return Pure(scope.Escape(report).As<v8::Value>());
}
}  // namespace node_webrtc
