#ifndef __RTCSTATSREPORT_H__
#define __RTCSTATSREPORT_H__

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

class RTCStatsReport
: public Nan::ObjectWrap
{

public:

  RTCStatsReport(webrtc::StatsReport* report);
  ~RTCStatsReport();

  //
  // Nodejs wrapping.
  //
  static void Init( Handle<Object> exports );
  static Nan::Persistent<Function> constructor;
  static NAN_METHOD(New);

  static NAN_METHOD(names);
  static NAN_METHOD(stat);

  static NAN_GETTER(GetTimestamp);
  static NAN_GETTER(GetType);

  static NAN_SETTER(ReadOnly);

private:

  webrtc::StatsReport* report;

};

}

#endif
