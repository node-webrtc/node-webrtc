/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "node.h"

#include "src/datachannel.h"
#include "src/errorfactory.h"
#include "src/mediastream.h"
#include "src/mediastreamtrack.h"
#include "src/rtcrtpreceiver.h"
#include "src/rtcrtpsender.h"
#include "src/rtcstatsreport.h"
#include "src/rtcstatsresponse.h"
#include "src/peerconnection.h"
#include "src/peerconnectionfactory.h"

using v8::Handle;
using v8::Object;

void dispose(void*) {
  node_webrtc::PeerConnectionFactory::Dispose();
}

void init(Handle<Object> exports, Handle<Object> module) {
  node_webrtc::ErrorFactory::Init(module);
  node_webrtc::PeerConnectionFactory::Init(exports);
  node_webrtc::PeerConnection::Init(exports);
  node_webrtc::DataChannel::Init(exports);
  node_webrtc::MediaStream::Init(exports);
  node_webrtc::MediaStreamTrack::Init(exports);
  node_webrtc::RTCRtpReceiver::Init(exports);
  node_webrtc::RTCRtpSender::Init(exports);
  node_webrtc::RTCStatsReport::Init(exports);
  node_webrtc::RTCStatsResponse::Init(exports);
  node::AtExit(dispose);
}

NODE_MODULE(wrtc, init)
