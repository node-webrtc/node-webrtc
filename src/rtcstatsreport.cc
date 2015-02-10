#include "rtcstatsreport.h"

using namespace node;
using namespace v8;

using namespace node_webrtc;

Persistent<Function> RTCStatsReport::constructor;

RTCStatsReport::RTCStatsReport(webrtc::StatsReport* report)
: report(report) {};

RTCStatsReport::~RTCStatsReport() {
  report = NULL;
}

NAN_METHOD(RTCStatsReport::New) {
  TRACE_CALL;
  NanScope();

  if (!args.IsConstructCall()) {
    return NanThrowTypeError("Use the new operator to construct the RTCStatsReport");
  }

  Local<External> _report = Local<External>::Cast(args[0]);
  webrtc::StatsReport* report = static_cast<webrtc::StatsReport*>(_report->Value());

  RTCStatsReport* obj = new RTCStatsReport(report);
  obj->Wrap( args.This() );

  TRACE_END;
  NanReturnValue( args.This() );
}

NAN_METHOD(RTCStatsReport::names) {
  TRACE_CALL;
  NanScope();

  RTCStatsReport* self = ObjectWrap::Unwrap<RTCStatsReport>( args.This() );

  std::vector<webrtc::StatsReport::Value> values = self->report->values;
  Local<Array> names = NanNew<Array>(values.size());
  for (std::vector<int>::size_type i = 0; i != values.size(); i++) {
    webrtc::StatsReport::Value value = values[i];
    std::string display_name = value.display_name();
    names->Set(i, NanNew<String>(display_name));
  }

  TRACE_END;
  NanReturnValue( names );
}

NAN_METHOD(RTCStatsReport::stat) {
  TRACE_CALL;
  NanScope();

  RTCStatsReport* self = ObjectWrap::Unwrap<RTCStatsReport>( args.This() );

  v8::String::Utf8Value _name(args[0]->ToString());
  std::string name = std::string(*_name);

  Local<Value> found = NanUndefined();
  std::vector<webrtc::StatsReport::Value> values = self->report->values;
  for (std::vector<int>::size_type i = 0; i != values.size(); i++) {
    webrtc::StatsReport::Value value = values[i];
    std::string display_name = std::string(value.display_name());
    if (display_name.compare(name) == 0) {
      found = NanNew<String>(value.value);
    }
  }

  TRACE_END;
  NanReturnValue( found );
}

NAN_GETTER(RTCStatsReport::GetTimestamp) {
  TRACE_CALL;
  NanScope();

  RTCStatsReport *self = ObjectWrap::Unwrap<RTCStatsReport>( args.Holder() );
  double timestamp = self->report->timestamp;

  TRACE_END;
  NanReturnValue( NanNew<Number>(timestamp) );
}

NAN_GETTER(RTCStatsReport::GetType) {
  TRACE_CALL;
  NanScope();

  RTCStatsReport *self = ObjectWrap::Unwrap<RTCStatsReport>( args.Holder() );
  std::string type = self->report->type;

  TRACE_END;
  NanReturnValue( NanNew<String>(type) );
}

NAN_SETTER(RTCStatsReport::ReadOnly) {
  INFO("RTCStatsReport::ReadOnly");
}

void RTCStatsReport::Init( Handle<Object> exports ) {
  Local<FunctionTemplate> tpl = NanNew<FunctionTemplate> ( New );
  tpl->SetClassName( NanNew( "RTCStatsReport" ) );
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  tpl->PrototypeTemplate()->Set( NanNew( "names" ),
    NanNew<FunctionTemplate>( names )->GetFunction() );
  tpl->PrototypeTemplate()->Set( NanNew( "stat" ),
    NanNew<FunctionTemplate>( stat )->GetFunction() );

  tpl->InstanceTemplate()->SetAccessor(NanNew("timestamp"), GetTimestamp, ReadOnly);
  tpl->InstanceTemplate()->SetAccessor(NanNew("type"), GetType, ReadOnly);

  NanAssignPersistent( constructor, tpl->GetFunction() );
  exports->Set( NanNew( "RTCStatsReport" ), tpl->GetFunction() );
}
