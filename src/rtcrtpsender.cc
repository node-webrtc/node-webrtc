/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "rtcrtpsender.h"

#include "converters/v8.h"
#include "converters/webrtc.h"
#include "functional/maybe.h"
#include "functional/operators.h"

using node_webrtc::AsyncObjectWrap;
using node_webrtc::Maybe;
using node_webrtc::MediaStreamTrack;
using node_webrtc::RTCRtpSender;
using v8::External;
using v8::Function;
using v8::FunctionTemplate;
using v8::Handle;
using v8::Local;
using v8::Object;
using v8::Value;

Nan::Persistent<Function> RTCRtpSender::constructor;

RTCRtpSender::RTCRtpSender(
    std::shared_ptr<node_webrtc::PeerConnectionFactory>&& factory,
    rtc::scoped_refptr<webrtc::RtpSenderInterface>&& sender,
    node_webrtc::MediaStreamTrack* track)
  : AsyncObjectWrap("RTCRtpSender")
  , _factory(std::move(factory))
  , _sender(std::move(sender))
  , _track(track) {
  if (_track) {
    _track->AddRef();
  }
}

RTCRtpSender::~RTCRtpSender() {
  if (_track) {
    _track->RemoveRef();
  }
}

NAN_METHOD(RTCRtpSender::New) {
  if (info.Length() != 2 || !info[0]->IsExternal() || !info[1]->IsExternal()) {
    return Nan::ThrowTypeError("You cannot construct a RTCRtpSender");
  }

  auto factory = *static_cast<std::shared_ptr<node_webrtc::PeerConnectionFactory>*>(Local<External>::Cast(info[0])->Value());
  auto sender = *static_cast<rtc::scoped_refptr<webrtc::RtpSenderInterface>*>(Local<External>::Cast(info[1])->Value());
  auto track = MediaStreamTrack::GetOrCreate(factory, sender->track());

  auto obj = new RTCRtpSender(std::move(factory), std::move(sender), track);
  obj->Wrap(info.This());

  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(RTCRtpSender::GetTrack) {
  (void) property;
  auto self = AsyncObjectWrap::Unwrap<RTCRtpSender>(info.Holder());
  Local<Value> result = Nan::Null();
  if (self->_track) {
    result = self->_track->ToObject();
  }
  info.GetReturnValue().Set(result);
}

NAN_GETTER(RTCRtpSender::GetTransport) {
  (void) property;
  info.GetReturnValue().Set(Nan::Null());
}

NAN_GETTER(RTCRtpSender::GetRtcpTransport) {
  (void) property;
  info.GetReturnValue().Set(Nan::Null());
}

NAN_METHOD(RTCRtpSender::GetCapabilities) {
  (void) info;
  Nan::ThrowError("Not yet implemented; file a feature request against node-webrtc");
}

NAN_METHOD(RTCRtpSender::GetParameters) {
  auto self = AsyncObjectWrap::Unwrap<RTCRtpSender>(info.Holder());
  auto parameters = self->_sender->GetParameters();
  CONVERT_OR_THROW_AND_RETURN(parameters, result, Local<Value>);
  info.GetReturnValue().Set(result);
}

NAN_METHOD(RTCRtpSender::SetParameters) {
  auto resolver = v8::Promise::Resolver::New(Nan::GetCurrentContext()->GetIsolate());
  resolver->Reject(Nan::Error("Not yet implemented; file a feature request against node-webrtc"));
  info.GetReturnValue().Set(resolver->GetPromise());
}

NAN_METHOD(RTCRtpSender::GetStats) {
  auto resolver = v8::Promise::Resolver::New(Nan::GetCurrentContext()->GetIsolate());
  resolver->Reject(Nan::Error("Not yet implemented; file a feature request against node-webrtc"));
  info.GetReturnValue().Set(resolver->GetPromise());
}

NAN_METHOD(RTCRtpSender::ReplaceTrack) {
  auto self = AsyncObjectWrap::Unwrap<RTCRtpSender>(info.Holder());
  auto resolver = v8::Promise::Resolver::New(Nan::GetCurrentContext()->GetIsolate());
  info.GetReturnValue().Set(resolver->GetPromise());
  CONVERT_ARGS_OR_REJECT_AND_RETURN(resolver, maybeTrack, node_webrtc::Either<node_webrtc::Null COMMA MediaStreamTrack*>);
  auto mediaStreamTrack = maybeTrack.FromEither<MediaStreamTrack*>([](Null) {
    return nullptr;
  }, [](MediaStreamTrack * track) {
    return track;
  });
  auto track = mediaStreamTrack ? mediaStreamTrack->track().get() : nullptr;
  if (self->_sender->SetTrack(track)) {
    if (self->_track) {
      self->_track->RemoveRef();
    }
    self->_track = mediaStreamTrack;
    if (mediaStreamTrack) {
      mediaStreamTrack->AddRef();
    }
    resolver->Resolve(Nan::Undefined());
  } else {
    resolver->Reject(Nan::Error("Failed to replaceTrack"));
  }
}

void RTCRtpSender::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("RTCRtpSender").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("track").ToLocalChecked(), GetTrack, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("transport").ToLocalChecked(), GetTransport, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("rtcpTransport").ToLocalChecked(), GetRtcpTransport, nullptr);
  Nan::SetMethod(tpl, "getCapabilities", GetCapabilities);
  Nan::SetPrototypeMethod(tpl, "getParameters", GetParameters);
  Nan::SetPrototypeMethod(tpl, "setParameters", SetParameters);
  Nan::SetPrototypeMethod(tpl, "getStats", GetStats);
  Nan::SetPrototypeMethod(tpl, "replaceTrack", ReplaceTrack);
  constructor.Reset(tpl->GetFunction());
  exports->Set(Nan::New("RTCRtpSender").ToLocalChecked(), tpl->GetFunction());
}
