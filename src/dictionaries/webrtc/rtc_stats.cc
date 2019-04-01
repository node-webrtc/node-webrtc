#include "src/dictionaries/webrtc/rtc_stats.h"

#include <iosfwd>
#include <string>
#include <vector>

#include <nan.h>
#include <v8.h>
#include <webrtc/api/stats/rtc_stats.h>

#include "src/converters.h"
#include "src/converters/v8.h"
#include "src/dictionaries/webrtc/rtc_stats_member_interface.h"  // IWYU pragma: keep
#include "src/functional/validation.h"

namespace node_webrtc {

TO_JS_IMPL(const webrtc::RTCStats*, value) {
  Nan::EscapableHandleScope scope;
  auto stats = Nan::New<v8::Object>();
  stats->Set(Nan::New("id").ToLocalChecked(), From<v8::Local<v8::Value>>(value->id()).UnsafeFromValid());
  stats->Set(Nan::New("timestamp").ToLocalChecked(), From<v8::Local<v8::Value>>(value->timestamp_us() / 1000.0).UnsafeFromValid());
  stats->Set(Nan::New("type").ToLocalChecked(), From<v8::Local<v8::Value>>(std::string(value->type())).UnsafeFromValid());
  for (const webrtc::RTCStatsMemberInterface* member : value->Members()) {
    if (member->is_defined()) {
      stats->Set(Nan::New(member->name()).ToLocalChecked(), From<v8::Local<v8::Value>>(member).UnsafeFromValid());
    }
  }
  return Pure(scope.Escape(stats).As<v8::Value>());
}

}  // namespace node_webrtc
