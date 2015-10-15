#include "rtcstatsreport.h"

using namespace node;
using namespace v8;

using namespace node_webrtc;

Nan::Persistent<Function> RTCStatsReport::constructor;

RTCStatsReport::RTCStatsReport(webrtc::StatsReport* report)
: report(report) {};

RTCStatsReport::~RTCStatsReport() {
  report = NULL;
}

NAN_METHOD(RTCStatsReport::New) {
  TRACE_CALL;
  Nan::HandleScope scope;

  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct the RTCStatsReport");
  }

  Local<External> _report = Local<External>::Cast(info[0]);
  webrtc::StatsReport* report = static_cast<webrtc::StatsReport*>(_report->Value());

  RTCStatsReport* obj = new RTCStatsReport(report);
  obj->Wrap( info.This() );

  TRACE_END;
  info.GetReturnValue().Set( info.This() );
}

NAN_METHOD(RTCStatsReport::names) {
  TRACE_CALL;
  Nan::HandleScope scope;

  RTCStatsReport* self = Nan::ObjectWrap::Unwrap<RTCStatsReport>( info.This() );

  std::vector<webrtc::StatsReport::Value> values = self->report->values;
  Local<Array> names = Nan::New<Array>(values.size());
  for (std::vector<int>::size_type i = 0; i != values.size(); i++) {
    webrtc::StatsReport::Value value = values[i];
    std::string display_name = value.display_name();
    names->Set(i, Nan::New<String>(display_name).ToLocalChecked());
  }

  TRACE_END;
  info.GetReturnValue().Set( names );
}

NAN_METHOD(RTCStatsReport::stat) {
  TRACE_CALL;
  Nan::HandleScope scope;

  RTCStatsReport* self = Nan::ObjectWrap::Unwrap<RTCStatsReport>( info.This() );

  v8::String::Utf8Value _name(info[0]->ToString());
  std::string name = std::string(*_name);

  Local<Value> found = Nan::Undefined();
  std::vector<webrtc::StatsReport::Value> values = self->report->values;
  for (std::vector<int>::size_type i = 0; i != values.size(); i++) {
    webrtc::StatsReport::Value value = values[i];
    std::string display_name = std::string(value.display_name());
    if (display_name.compare(name) == 0) {
      found = Nan::New<String>(value.value).ToLocalChecked();
    }
  }

  TRACE_END;
  info.GetReturnValue().Set( found );
}

NAN_GETTER(RTCStatsReport::GetTimestamp) {
  TRACE_CALL;
  Nan::HandleScope scope;

  RTCStatsReport *self = Nan::ObjectWrap::Unwrap<RTCStatsReport>( info.Holder() );
  double timestamp = self->report->timestamp;

  TRACE_END;
  info.GetReturnValue().Set( Nan::New<Number>(timestamp) );
}

NAN_GETTER(RTCStatsReport::GetType) {
  TRACE_CALL;
  Nan::HandleScope scope;

  RTCStatsReport *self = Nan::ObjectWrap::Unwrap<RTCStatsReport>( info.Holder() );
  std::string type = self->report->type;

  TRACE_END;
  info.GetReturnValue().Set( Nan::New<String>(type).ToLocalChecked() );
}

NAN_SETTER(RTCStatsReport::ReadOnly) {
  INFO("RTCStatsReport::ReadOnly");
}

void RTCStatsReport::Init( Handle<Object> exports ) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate> ( New );
  tpl->SetClassName( Nan::New( "RTCStatsReport" ).ToLocalChecked() );
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  tpl->PrototypeTemplate()->Set( Nan::New( "names" ).ToLocalChecked(),
    Nan::New<FunctionTemplate>( names )->GetFunction() );
  tpl->PrototypeTemplate()->Set( Nan::New( "stat" ).ToLocalChecked(),
    Nan::New<FunctionTemplate>( stat )->GetFunction() );

  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("timestamp").ToLocalChecked(), GetTimestamp, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("type").ToLocalChecked(), GetType, ReadOnly);

   constructor.Reset(tpl->GetFunction() );
  exports->Set( Nan::New( "RTCStatsReport" ).ToLocalChecked(), tpl->GetFunction() );
}
