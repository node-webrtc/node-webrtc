/* Copyright (c) 2017 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "rtcstatsreport.h"

#include <string>
#include <vector>

#include "common.h"

using node_webrtc::RTCStatsReport;
using v8::Array;
using v8::External;
using v8::Function;
using v8::FunctionTemplate;
using v8::Handle;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::String;
using v8::Value;

Nan::Persistent<Function> RTCStatsReport::constructor;

RTCStatsReport::RTCStatsReport(webrtc::StatsReport* report)
: report(report) {}

RTCStatsReport::~RTCStatsReport() {
  report = nullptr;
}

NAN_METHOD(RTCStatsReport::New) {
  TRACE_CALL;

  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct the RTCStatsReport");
  }

  Local<External> _report = Local<External>::Cast(info[0]);
  webrtc::StatsReport* report = static_cast<webrtc::StatsReport*>(_report->Value());

  RTCStatsReport* obj = new RTCStatsReport(report);
  obj->Wrap(info.This());

  TRACE_END;
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(RTCStatsReport::names) {
  TRACE_CALL;
  Nan::HandleScope scope;

  RTCStatsReport* self = Nan::ObjectWrap::Unwrap<RTCStatsReport>(info.This());

  webrtc::StatsReport::Values values = self->report->values();
  Local<Array> names = Nan::New<Array>(values.size());
  size_t i = 0;
  for (auto const &pair : values) {
    const rtc::linked_ptr<webrtc::StatsReport::Value> value = pair.second;
    std::string display_name = value->display_name();
    names->Set(i++, Nan::New<String>(display_name).ToLocalChecked());
  }

  TRACE_END;
  info.GetReturnValue().Set(names);
}

NAN_METHOD(RTCStatsReport::stat) {
  TRACE_CALL;
  Nan::HandleScope scope;

  RTCStatsReport* self = Nan::ObjectWrap::Unwrap<RTCStatsReport>(info.This());

  v8::String::Utf8Value _name(info[0]->ToString());
  std::string name = std::string(*_name);

  Local<Value> found = Nan::Undefined();
  webrtc::StatsReport::Values values = self->report->values();
  for (auto const &pair : values) {
    const rtc::linked_ptr<webrtc::StatsReport::Value> value = pair.second;
    std::string display_name = std::string(value->display_name());
    if (display_name.compare(name) == 0) {
      found = Nan::New<String>(value->ToString()).ToLocalChecked();
    }
  }

  TRACE_END;
  info.GetReturnValue().Set(found);
}

NAN_GETTER(RTCStatsReport::GetTimestamp) {
  TRACE_CALL;
  Nan::HandleScope scope;

  RTCStatsReport *self = Nan::ObjectWrap::Unwrap<RTCStatsReport>(info.Holder());
  double timestamp = self->report->timestamp();

  TRACE_END;
  info.GetReturnValue().Set(Nan::New<Number>(timestamp));
}

NAN_GETTER(RTCStatsReport::GetType) {
  TRACE_CALL;
  Nan::HandleScope scope;

  RTCStatsReport *self = Nan::ObjectWrap::Unwrap<RTCStatsReport>(info.Holder());
  std::string type = self->report->TypeToString();

  TRACE_END;
  info.GetReturnValue().Set(Nan::New<String>(type).ToLocalChecked());
}

NAN_SETTER(RTCStatsReport::ReadOnly) {
  INFO("RTCStatsReport::ReadOnly");
}

void RTCStatsReport::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate> (New);
  tpl->SetClassName(Nan::New("RTCStatsReport").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "names", names);
  Nan::SetPrototypeMethod(tpl, "stat", stat);

  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("timestamp").ToLocalChecked(), GetTimestamp, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("type").ToLocalChecked(), GetType, ReadOnly);

  constructor.Reset(tpl->GetFunction());
  exports->Set(Nan::New("RTCStatsReport").ToLocalChecked(), tpl->GetFunction());
}
