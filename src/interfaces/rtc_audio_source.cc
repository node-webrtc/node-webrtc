/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/rtc_audio_source.h"

#include <webrtc/api/peer_connection_interface.h>
#include <webrtc/rtc_base/ref_counted_object.h>

#include "src/converters.h"
#include "src/converters/arguments.h"
#include "src/node/error.h"
#include "src/functional/maybe.h"
#include "src/interfaces/media_stream_track.h"

namespace node_webrtc {

Nan::Persistent<v8::Function>& RTCAudioSource::constructor() {
  static Nan::Persistent<v8::Function> constructor;
  return constructor;
}

RTCAudioSource::RTCAudioSource() {
  _source = new rtc::RefCountedObject<RTCAudioTrackSource>();
}

NAN_METHOD(RTCAudioSource::New) {
  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct an RTCAudioSource.");
  }

  auto instance = new RTCAudioSource();
  instance->Wrap(info.This());

  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(RTCAudioSource::CreateTrack) {
  auto self = Nan::ObjectWrap::Unwrap<RTCAudioSource>(info.Holder());

  // TODO(mroberts): Again, we have some implicit factory we are threading around. How to handle?
  auto factory = PeerConnectionFactory::GetOrCreateDefault();
  auto track = factory->factory()->CreateAudioTrack(rtc::CreateRandomUuid(), self->_source);
  auto result = MediaStreamTrack::wrap()->GetOrCreate(factory, track);

  info.GetReturnValue().Set(result->ToObject());
}

NAN_METHOD(RTCAudioSource::OnData) {
  auto self = Nan::ObjectWrap::Unwrap<RTCAudioSource>(info.Holder());
  CONVERT_ARGS_OR_THROW_AND_RETURN(dict, RTCOnDataEventDict)
  self->_source->PushData(dict);
}

void RTCAudioSource::Init(v8::Handle<v8::Object> exports) {
  auto tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("RTCAudioSource").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "createTrack", CreateTrack);
  Nan::SetPrototypeMethod(tpl, "onData", OnData);

  constructor().Reset(tpl->GetFunction());
  exports->Set(Nan::New("RTCAudioSource").ToLocalChecked(), tpl->GetFunction());
}

}  // namespace node_webrtc
