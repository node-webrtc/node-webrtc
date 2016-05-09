#include "rtcstatsresponse.h"

#include <vector>

#include "common.h"
#include "rtcstatsreport.h"

using node_webrtc::RTCStatsResponse;
using v8::Array;
using v8::External;
using v8::Function;
using v8::FunctionTemplate;
using v8::Handle;
using v8::Local;
using v8::Object;
using v8::Value;

Nan::Persistent<Function> RTCStatsResponse::constructor;

NAN_METHOD(RTCStatsResponse::New) {
  TRACE_CALL;

  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct the RTCStatsResponse");
  }

  Local<External> _reports = Local<External>::Cast(info[0]);
  webrtc::StatsReports* reports = static_cast<webrtc::StatsReports*>(_reports->Value());

  RTCStatsResponse* obj = new RTCStatsResponse(*reports);
  obj->Wrap(info.This());

  TRACE_END;
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(RTCStatsResponse::result) {
  TRACE_CALL;

  RTCStatsResponse* self = Nan::ObjectWrap::Unwrap<RTCStatsResponse>(info.This());

  Local<Array> reports = Nan::New<Array>(self->reports.size());
  for (std::vector<int>::size_type i = 0; i != self->reports.size(); i++) {
    const void *copy = static_cast<const void*>(self->reports.at(i));
    Local<Value> cargv[1];
    cargv[0] = Nan::New<External>(const_cast<void*>(copy));
    reports->Set(i, Nan::New(RTCStatsReport::constructor)->NewInstance(1, cargv));
  }

  TRACE_END;
  info.GetReturnValue().Set(reports);
}

void RTCStatsResponse::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate> (New);
  tpl->SetClassName(Nan::New("RTCStatsResponse").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Nan::SetPrototypeMethod(tpl, "result", result);
  constructor.Reset(tpl->GetFunction());
  exports->Set(Nan::New("RTCStatsResponse").ToLocalChecked(), tpl->GetFunction());
}
