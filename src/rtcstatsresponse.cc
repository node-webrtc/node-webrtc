#include "rtcstatsresponse.h"
#include "rtcstatsreport.h"

using namespace node;
using namespace v8;

using namespace node_webrtc;

Nan::Persistent<Function> RTCStatsResponse::constructor;

NAN_METHOD(RTCStatsResponse::New) {
  TRACE_CALL;
  Nan::HandleScope scope;

  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct the RTCStatsResponse");
  }

  Local<External> _reports = Local<External>::Cast(info[0]);
  webrtc::StatsReports* reports = static_cast<webrtc::StatsReports*>(_reports->Value());

  RTCStatsResponse* obj = new RTCStatsResponse(*reports);
  obj->Wrap( info.This() );

  TRACE_END;
  info.GetReturnValue().Set( info.This() );
}

NAN_METHOD(RTCStatsResponse::result) {
  TRACE_CALL;
  Nan::HandleScope scope;

  RTCStatsResponse* self = Nan::ObjectWrap::Unwrap<RTCStatsResponse>( info.This() );

  Local<Array> reports = Nan::New<Array>(self->reports.size());
  for (std::vector<int>::size_type i = 0; i != self->reports.size(); i++) {
    void *copy = (void *) self->reports.at(i);
    v8::Local<v8::Value> cargv[1];
    cargv[0] = Nan::New<v8::External>(copy);
    reports->Set(i, Nan::New(RTCStatsReport::constructor)->NewInstance(1, cargv));
  }

  TRACE_END;
  info.GetReturnValue().Set( reports );
}

void RTCStatsResponse::Init( Handle<Object> exports ) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate> ( New );
  tpl->SetClassName( Nan::New( "RTCStatsResponse" ).ToLocalChecked() );
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->PrototypeTemplate()->Set( Nan::New( "result" ).ToLocalChecked(),
    Nan::New<FunctionTemplate>( result )->GetFunction() );
   constructor.Reset(tpl->GetFunction() );
  exports->Set( Nan::New( "RTCStatsResponse" ).ToLocalChecked(), tpl->GetFunction() );
}
