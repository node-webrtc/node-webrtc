/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/events.h"

#include "src/datachannel.h"
#include "src/peerconnection.h"
#include "src/rtcaudiosink.h"
#include "src/rtcvideosink.h"

using node_webrtc::AddIceCandidateEvent;
using node_webrtc::DataChannel;
using node_webrtc::DataChannelEvent;
using node_webrtc::DataChannelStateChangeEvent;
using node_webrtc::ErrorEvent;
using node_webrtc::IceConnectionStateChangeEvent;
using node_webrtc::IceGatheringStateChangeEvent;
using node_webrtc::IceEvent;
using node_webrtc::MessageEvent;
using node_webrtc::NegotiationNeededEvent;
using node_webrtc::OnAddTrackEvent;
using node_webrtc::OnDataEvent;
using node_webrtc::OnFrameEvent;
using node_webrtc::PeerConnection;
using node_webrtc::PromiseEvent;
using node_webrtc::RTCAudioSink;
using node_webrtc::RTCVideoSink;
using node_webrtc::SignalingStateChangeEvent;

void DataChannelEvent::Dispatch(PeerConnection& peerConnection) {
  peerConnection.HandleDataChannelEvent(*this);
}

void DataChannelStateChangeEvent::Dispatch(DataChannel& dataChannel) {
  dataChannel.HandleStateEvent(*this);
}

// NOTE(mroberts): https://stackoverflow.com/a/25594741
namespace node_webrtc {

template <>
void ErrorEvent<DataChannel>::Dispatch(DataChannel& dataChannel) {
  dataChannel.HandleErrorEvent(*this);
}

}  // namespace node_webrtc

void MessageEvent::Dispatch(DataChannel& dataChannel) {
  dataChannel.HandleMessageEvent(*this);
}

void NegotiationNeededEvent::Dispatch(PeerConnection& peerConnection) {
  peerConnection.HandleNegotiationNeededEvent(*this);
}

void OnAddTrackEvent::Dispatch(PeerConnection& peerConnection) {
  peerConnection.HandleOnAddTrackEvent(*this);
}

void IceEvent::Dispatch(PeerConnection& peerConnection) {
  peerConnection.HandleIceCandidateEvent(*this);
}

void AddIceCandidateEvent::Dispatch(PeerConnection& peerConnection) {
  peerConnection.HandleAddIceCandidateEvent(*this);
  PromiseEvent<PeerConnection>::Dispatch(peerConnection);
}

void IceConnectionStateChangeEvent::Dispatch(PeerConnection& peerConnection) {
  peerConnection.HandleIceConnectionStateChangeEvent(*this);
}

void IceGatheringStateChangeEvent::Dispatch(PeerConnection& peerConnection) {
  peerConnection.HandleIceGatheringStateChangeEvent(*this);
}

void SignalingStateChangeEvent::Dispatch(PeerConnection& peerConnection) {
  peerConnection.HandleSignalingStateChangeEvent(*this);
}

void OnFrameEvent::Dispatch(RTCVideoSink& sink) {
  sink.HandleOnFrameEvent(*this);
}

void OnDataEvent::Dispatch(RTCAudioSink& sink) {
  sink.HandleOnDataEvent(*this);
}
