/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/legacyrtcstatsreport.h"

#include <nan.h>
#include <v8.h>

#include "src/common.h"

using node_webrtc::LegacyStatsReport;
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

Nan::Persistent<Function>& LegacyStatsReport::constructor() {
  static Nan::Persistent<Function> constructor;
  return constructor;
}

NAN_METHOD(LegacyStatsReport::New) {
  TRACE_CALL;

  if (info.Length() != 2 || !info[0]->IsExternal() || !info[1]->IsExternal()) {
    return Nan::ThrowTypeError("You cannot construct an LegacyStatsReport");
  }

  auto timestamp = static_cast<double*>(Local<External>::Cast(info[0])->Value());
  auto stats = static_cast<std::map<std::string, std::string>*>(Local<External>::Cast(info[1])->Value());

  auto obj = new LegacyStatsReport(*timestamp, *stats);
  obj->Wrap(info.This());

  TRACE_END;
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(LegacyStatsReport::names) {
  TRACE_CALL;
  Nan::HandleScope scope;

  auto self = Nan::ObjectWrap::Unwrap<LegacyStatsReport>(info.This());
  Local<Array> names = Nan::New<Array>(self->_stats.size());

  uint32_t i = 0;
  for (auto pair : self->_stats) {
    auto name = pair.first;
    names->Set(i++, Nan::New<String>(name).ToLocalChecked());
  }

  TRACE_END;
  info.GetReturnValue().Set(names);
}

NAN_METHOD(LegacyStatsReport::stat) {
  TRACE_CALL;
  Nan::HandleScope scope;

  auto self = Nan::ObjectWrap::Unwrap<LegacyStatsReport>(info.This());
  auto requested = std::string(*v8::String::Utf8Value(info[0]->ToString()));

  Local<Value> found = Nan::Undefined();
  for (auto pair : self->_stats) {
    auto name = pair.first;
    if (requested == name) {
      found = Nan::New<String>(pair.second).ToLocalChecked();
    }
  }

  TRACE_END;
  info.GetReturnValue().Set(found);
}

NAN_GETTER(LegacyStatsReport::GetTimestamp) {
  TRACE_CALL;
  (void) property;

  auto self = Nan::ObjectWrap::Unwrap<LegacyStatsReport>(info.Holder());

  TRACE_END;
  info.GetReturnValue().Set(Nan::New<Number>(self->_timestamp));
}

NAN_GETTER(LegacyStatsReport::GetType) {
  TRACE_CALL;
  (void) property;

  auto self = Nan::ObjectWrap::Unwrap<LegacyStatsReport>(info.Holder());
  auto type = self->_stats.find("type")->second;

  TRACE_END;
  info.GetReturnValue().Set(Nan::New<String>(type).ToLocalChecked());
}

NAN_SETTER(LegacyStatsReport::ReadOnly) {
  (void) info;
  (void) property;
  (void) value;
  INFO("LegacyStatsReport::ReadOnly");
}

LegacyStatsReport* LegacyStatsReport::Create(double timestamp, const std::map<std::string, std::string>& stats) {
  Nan::HandleScope scope;
  Local<Value> cargv[2];
  cargv[0] = Nan::New<External>(static_cast<void*>(&timestamp));
  cargv[1] = Nan::New<External>(const_cast<void*>(static_cast<const void*>(&stats)));
  auto report = Nan::NewInstance(Nan::New(LegacyStatsReport::constructor()), 2, cargv).ToLocalChecked();
  return Nan::ObjectWrap::Unwrap<LegacyStatsReport>(report);
}

void LegacyStatsReport::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate> (New);
  tpl->SetClassName(Nan::New("LegacyStatsReport").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "names", names);
  Nan::SetPrototypeMethod(tpl, "stat", stat);

  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("timestamp").ToLocalChecked(), GetTimestamp, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("type").ToLocalChecked(), GetType, ReadOnly);

  constructor().Reset(tpl->GetFunction());
  exports->Set(Nan::New("LegacyStatsReport").ToLocalChecked(), tpl->GetFunction());
}
