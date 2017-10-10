/* Copyright (c) 2017 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/events.h"

#include <nan.h>

#include "src/datachannel.h"
#include "src/peerconnection.h"

using v8::Local;
using v8::Object;
using v8::Value;
using node_webrtc::Event;
using node_webrtc::DataChannel;
using node_webrtc::DataChannelEvent;
using node_webrtc::DataChannelStateChangeEvent;
using node_webrtc::ErrorEvent;
using node_webrtc::GetStatsEvent;
using node_webrtc::IceConnectionStateChangeEvent;
using node_webrtc::IceGatheringStateChangeEvent;
using node_webrtc::IceEvent;
using node_webrtc::MessageEvent;
using node_webrtc::PeerConnection;
using node_webrtc::SdpEvent;
using node_webrtc::SignalingStateChangeEvent;

void DataChannelEvent::Dispatch(PeerConnection& peerConnection) {
  peerConnection.HandleDataChannelEvent(*this);
}

void DataChannelStateChangeEvent::Dispatch(DataChannel& dataChannel) {
  dataChannel.HandleStateEvent(*this);
}

template <>
void ErrorEvent<DataChannel>::Dispatch(DataChannel& dataChannel) {
  dataChannel.HandleErrorEvent(*this);
}

void MessageEvent::Dispatch(DataChannel& dataChannel) {
  dataChannel.HandleMessageEvent(*this);
}

void GetStatsEvent::Dispatch(PeerConnection &peerConnection) {
  peerConnection.HandleGetStatsEvent(*this);
}

void IceEvent::Dispatch(PeerConnection& peerConnection) {
  peerConnection.HandleIceCandidateEvent(*this);
}

void IceConnectionStateChangeEvent::Dispatch(PeerConnection& peerConnection) {
  peerConnection.HandleIceConnectionStateChangeEvent(*this);
}

void IceGatheringStateChangeEvent::Dispatch(PeerConnection& peerConnection) {
  peerConnection.HandleIceGatheringStateChangeEvent(*this);
}

void SdpEvent::Dispatch(PeerConnection& peerConnection) {
  Nan::HandleScope scope;

  auto resolver = (*_resolver).Get(Nan::GetCurrentContext()->GetIsolate());

  // TODO(mroberts): Handle error scenarios.
  Local<Value> result = Nan::Null();

  std::string sdp;
  if (_sdp->ToString(&sdp)) {
    auto type = _sdp->type();
    Local<Object> description = Nan::New<Object>();
    Nan::Set(description, Nan::New("type").ToLocalChecked(), Nan::New(type).ToLocalChecked());
    Nan::Set(description, Nan::New("sdp").ToLocalChecked(), Nan::New(sdp).ToLocalChecked());
    result = description;
  }

  resolver->Resolve(result);
}

void SignalingStateChangeEvent::Dispatch(PeerConnection& peerConnection) {
  peerConnection.HandleSignalingStateChangeEvent(*this);
}
