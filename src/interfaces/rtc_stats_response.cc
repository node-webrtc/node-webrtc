/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/rtc_stats_response.h"

#include <cstdint>

#include <nan.h>
#include <node-addon-api/napi.h>
#include <v8.h>

#include "src/converters/napi.h"
#include "src/interfaces/legacy_rtc_stats_report.h"

namespace node_webrtc {

Nan::Persistent<v8::Function>& RTCStatsResponse::constructor() {
  static Nan::Persistent<v8::Function> constructor;
  return constructor;
}

NAN_METHOD(RTCStatsResponse::New) {
  if (info.Length() != 2 || !info[0]->IsExternal() || !info[1]->IsExternal()) {
    return Nan::ThrowTypeError("You cannot construct an RTCStatsResponse");
  }

  auto timestamp = static_cast<double*>(v8::Local<v8::External>::Cast(info[0])->Value());
  auto reports = static_cast<std::vector<std::map<std::string, std::string>>*>(v8::Local<v8::External>::Cast(info[1])->Value());

  auto obj = new RTCStatsResponse(*timestamp, *reports);
  obj->Wrap(info.This());

  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(RTCStatsResponse::result) {
  auto self = Nan::ObjectWrap::Unwrap<RTCStatsResponse>(info.This());
  auto timestamp = self->_timestamp;
  auto reports = Nan::New<v8::Array>(self->_reports.size());

  uint32_t i = 0;
  for (auto const& stats : self->_reports) {
    auto report = LegacyStatsReport::Create(timestamp, stats);
    reports->Set(i++, napi::UnsafeToV8(report->Value()));
  }

  info.GetReturnValue().Set(reports);
}

RTCStatsResponse* RTCStatsResponse::Create(
    double timestamp,
    const std::vector<std::map<std::string, std::string>>& reports) {
  Nan::HandleScope scope;
  v8::Local<v8::Value> cargv[2];
  cargv[0] = Nan::New<v8::External>(static_cast<void*>(&timestamp));
  cargv[1] = Nan::New<v8::External>(const_cast<void*>(static_cast<const void*>(&reports)));
  auto response = Nan::NewInstance(Nan::New(RTCStatsResponse::constructor()), 2, cargv).ToLocalChecked();
  return Nan::ObjectWrap::Unwrap<RTCStatsResponse>(response);
}

void RTCStatsResponse::Init(v8::Handle<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate> (New);
  tpl->SetClassName(Nan::New("RTCStatsResponse").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Nan::SetPrototypeMethod(tpl, "result", result);
  constructor().Reset(tpl->GetFunction());
  exports->Set(Nan::New("RTCStatsResponse").ToLocalChecked(), tpl->GetFunction());
}

}  // namespace node_webrtc
