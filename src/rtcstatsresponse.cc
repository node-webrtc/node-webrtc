/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/rtcstatsresponse.h"

#include <nan.h>
#include <v8.h>  // IWYU pragma: keep

#include "src/common.h"
#include "src/legacyrtcstatsreport.h"  // IWYU pragma: keep

using node_webrtc::RTCStatsResponse;
using v8::Array;
using v8::External;
using v8::Function;
using v8::FunctionTemplate;
using v8::Handle;
using v8::Local;
using v8::Object;
using v8::Value;

Nan::Persistent<Function>& RTCStatsResponse::constructor() {
  static Nan::Persistent<Function> constructor;
  return constructor;
}

NAN_METHOD(RTCStatsResponse::New) {
  TRACE_CALL;

  if (info.Length() != 2 || !info[0]->IsExternal() || !info[1]->IsExternal()) {
    return Nan::ThrowTypeError("You cannot construct an RTCStatsResponse");
  }

  auto timestamp = static_cast<double*>(Local<External>::Cast(info[0])->Value());
  auto reports = static_cast<std::vector<std::map<std::string, std::string>>*>(Local<External>::Cast(info[1])->Value());

  auto obj = new RTCStatsResponse(*timestamp, *reports);
  obj->Wrap(info.This());

  TRACE_END;
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(RTCStatsResponse::result) {
  TRACE_CALL;

  auto self = Nan::ObjectWrap::Unwrap<RTCStatsResponse>(info.This());
  auto timestamp = self->_timestamp;
  auto reports = Nan::New<Array>(self->_reports.size());

  uint32_t i = 0;
  for (auto const& stats : self->_reports) {
    auto report = LegacyStatsReport::Create(timestamp, stats);
    reports->Set(i++, report->handle());
  }

  TRACE_END;
  info.GetReturnValue().Set(reports);
}

RTCStatsResponse* RTCStatsResponse::Create(
    double timestamp,
    const std::vector<std::map<std::string, std::string>>& reports) {
  Nan::HandleScope scope;
  Local<Value> cargv[2];
  cargv[0] = Nan::New<External>(static_cast<void*>(&timestamp));
  cargv[1] = Nan::New<External>(const_cast<void*>(static_cast<const void*>(&reports)));
  auto response = Nan::NewInstance(Nan::New(RTCStatsResponse::constructor()), 2, cargv).ToLocalChecked();
  return Nan::ObjectWrap::Unwrap<RTCStatsResponse>(response);
}

void RTCStatsResponse::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate> (New);
  tpl->SetClassName(Nan::New("RTCStatsResponse").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Nan::SetPrototypeMethod(tpl, "result", result);
  constructor().Reset(tpl->GetFunction());
  exports->Set(Nan::New("RTCStatsResponse").ToLocalChecked(), tpl->GetFunction());
}
