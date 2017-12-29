/* Copyright (c) 2017 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/events/peerconnectionevents.h"

#include "src/datachannel.h"
#include "src/peerconnection.h"

using node_webrtc::DataChannelEvent;
using node_webrtc::GetStatsEvent;
using node_webrtc::IceEvent;
using node_webrtc::IceConnectionStateChangeEvent;
using node_webrtc::IceGatheringStateChangeEvent;
using node_webrtc::PeerConnection;
using node_webrtc::SdpEvent;
using node_webrtc::SignalingStateChangeEvent;
using v8::Local;
using v8::Object;
using v8::Value;

void DataChannelEvent::Dispatch(PeerConnection& peerConnection) {
  peerConnection.HandleDataChannelEvent(*this);
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

void SdpEvent::Dispatch(PeerConnection&) {
  Nan::HandleScope scope;

  auto resolver = Nan::New(*_resolver);

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
