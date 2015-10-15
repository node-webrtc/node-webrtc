#ifndef __RTCSTATSRESPONSE_H__
#define __RTCSTATSRESPONSE_H__

#include <node.h>
#include <v8.h>
#include <node_object_wrap.h>
#include <uv.h>

#include "talk/app/webrtc/statstypes.h"

#include "common.h"
#include "nan.h"

using namespace node;
using namespace v8;

namespace node_webrtc {

class RTCStatsResponse
: public Nan::ObjectWrap
{

public:

  RTCStatsResponse(webrtc::StatsReports reports): reports(reports) {};
  ~RTCStatsResponse() {};

  //
  // Nodejs wrapping.
  //
  static void Init( Handle<Object> exports );
  static Nan::Persistent<Function> constructor;
  static NAN_METHOD(New);

  static NAN_METHOD(result);

private:

  webrtc::StatsReports reports;

};

}

#endif
