#ifndef SRC_RTCSTATSREPORT_H_
#define SRC_RTCSTATSREPORT_H_

#include "nan.h"
#include "v8.h"  // IWYU pragma: keep

#include "webrtc/api/statstypes.h"  // IWYU pragma: keep

namespace node_webrtc {

class RTCStatsReport
: public Nan::ObjectWrap {
 public:
  explicit RTCStatsReport(webrtc::StatsReport* report);
  ~RTCStatsReport();

  //
  // Nodejs wrapping.
  //
  static void Init(v8::Handle<v8::Object> exports);
  static Nan::Persistent<v8::Function> constructor;
  static NAN_METHOD(New);

  static NAN_METHOD(names);
  static NAN_METHOD(stat);

  static NAN_GETTER(GetTimestamp);
  static NAN_GETTER(GetType);

  static NAN_SETTER(ReadOnly);

 private:
  webrtc::StatsReport* report;
};

}  // namespace node_webrtc

#endif  // SRC_RTCSTATSREPORT_H_
