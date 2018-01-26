/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "mediadevices.h"

#include "common.h"
#include "mediastream.h"
#include "create-offer-observer.h"

#include <iostream>

using node_webrtc::MediaDevices;
using v8::External;
using v8::Function;
using v8::FunctionTemplate;
using v8::Handle;
using v8::Integer;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::String;
using v8::Uint32;
using v8::Value;

Nan::Persistent<Function> MediaDevices::constructor;


// MediaDevices constructor/destructor
MediaDevices::MediaDevices() {
  _peer_connection_factory = PeerConnectionFactory::GetOrCreateDefault();

  ASSERT(_peer_connection_factory.get() != NULL);
}

MediaDevices::~MediaDevices() {
  TRACE_CALL;
  _peer_connection_factory = nullptr;
  TRACE_END;
}

// NodeJS Wrapping
NAN_METHOD(MediaDevices::New) {
  TRACE_CALL;

  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct the MediaDevices.");
  }

  MediaDevices* obj = new MediaDevices();
  obj->Wrap(info.This());

  TRACE_END;
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(MediaDevices::GetUserMedia) {
  TRACE_CALL;

  MediaDevices* self = Nan::ObjectWrap::Unwrap<MediaDevices>(info.This());

  const char kAudioLabel[] = "audio_label";
//  const char kVideoLabel[] = "video_label";
  const char kStreamLabel[] = "stream_label";

  rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
      self->_peer_connection_factory->factory()->CreateAudioTrack(
          kAudioLabel, self->_peer_connection_factory->factory()->CreateAudioSource(NULL)));

//  rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track(
//      self->_peer_connection_factory->CreateVideoTrack(
//          kVideoLabel,
//          self->_peer_connection_factory->CreateVideoSource(OpenVideoCaptureDevice(),
//                                                      NULL)));

  rtc::scoped_refptr<webrtc::MediaStreamInterface> stream =
      self->_peer_connection_factory->factory()->CreateLocalMediaStream(kStreamLabel);

  stream->AddTrack(audio_track);
//  stream->AddTrack(video_track);

  Local<Value> cargv[1];
  cargv[0] = Nan::New<External>(static_cast<void*>(stream));
  Local<Value> obj = Nan::New(MediaStream::constructor)->NewInstance(1, cargv);

  info.GetReturnValue().Set(obj);

  TRACE_END;
}

void MediaDevices::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("MediaDevices").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "getUserMedia", GetUserMedia);

  constructor.Reset(tpl->GetFunction());
  exports->Set(Nan::New("MediaDevices").ToLocalChecked(), tpl->GetFunction());
}
