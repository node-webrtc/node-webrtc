#ifndef SRC_RTCSTATSRESPONSE_H_
#define SRC_RTCSTATSRESPONSE_H_

#include "nan.h"
#include "v8.h"  // IWYU pragma: keep

#include "webrtc/api/statstypes.h"

namespace node_webrtc {

class RTCStatsResponse
: public Nan::ObjectWrap {
 public:
  explicit RTCStatsResponse(webrtc::StatsReports reports): reports(reports) {}
  ~RTCStatsResponse() {}

  //
  // Nodejs wrapping.
  //
  static void Init(v8::Handle<v8::Object> exports);
  static Nan::Persistent<v8::Function> constructor;
  static NAN_METHOD(New);

  static NAN_METHOD(result);

 private:
  webrtc::StatsReports reports;
};

}  // namespace node_webrtc

#endif  // SRC_RTCSTATSRESPONSE_H_
