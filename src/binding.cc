/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "node.h"

#include "datachannel.h"
#include "rtcstatsreport.h"
#include "rtcstatsresponse.h"
#include "peerconnection.h"
#include "peerconnectionfactory.h"
#include "fileaudiodevicefactory.h"
#include "mediadevices.h"
#include "mediastreamtrack.h"
#include "mediastream.h"

using v8::Handle;
using v8::Object;

void dispose(void*) {
  node_webrtc::PeerConnectionFactory::Dispose();
}

void init(Handle<Object> exports) {
  node_webrtc::FileAudioDeviceFactory::Init(exports);
  node_webrtc::PeerConnectionFactory::Init(exports);
  node_webrtc::PeerConnection::Init(exports);
  node_webrtc::DataChannel::Init(exports);
  node_webrtc::RTCStatsReport::Init(exports);
  node_webrtc::RTCStatsResponse::Init(exports);
  node_webrtc::MediaDevices::Init(exports);
  node_webrtc::MediaStreamTrack::Init(exports);
  node_webrtc::MediaStream::Init(exports);
  node::AtExit(dispose);
}

NODE_MODULE(wrtc, init)
