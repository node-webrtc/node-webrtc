/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include <node.h>
#include <v8.h>  // IWYU pragma: keep

#include "src/datachannel.h"  // IWYU pragma: keep
#include "src/errorfactory.h"  // IWYU pragma: keep
#include "src/getusermedia.h"  // IWYU pragma: keep
#include "src/i420helpers.h"  // IWYU pragma: keep
#include "src/legacyrtcstatsreport.h"  // IWYU pragma: keep
#include "src/mediastream.h"  // IWYU pragma: keep
#include "src/mediastreamtrack.h"  // IWYU pragma: keep
#include "src/peerconnection.h"  // IWYU pragma: keep
#include "src/peerconnectionfactory.h"
#include "src/rtcrtpreceiver.h"  // IWYU pragma: keep
#include "src/rtcrtpsender.h"  // IWYU pragma: keep
#include "src/rtcrtptransceiver.h"  // IWYU pragma: keep
#include "src/rtcstatsresponse.h"  // IWYU pragma: keep
#include "src/rtcvideosink.h"  // IWYU pragma: keep
#include "src/rtcvideosource.h"  // IWYU pragma: keep

static void dispose(void*) {
  node_webrtc::PeerConnectionFactory::Dispose();
}

static void init(v8::Handle<v8::Object> exports, v8::Handle<v8::Object> module) {
  node_webrtc::ErrorFactory::Init(module);
  node_webrtc::GetUserMedia::Init(exports);
  node_webrtc::I420Helpers::Init(exports);
  node_webrtc::PeerConnectionFactory::Init(exports);
  node_webrtc::PeerConnection::Init(exports);
  node_webrtc::DataChannel::Init(exports);
  node_webrtc::MediaStream::Init(exports);
  node_webrtc::MediaStreamTrack::Init(exports);
  node_webrtc::RTCRtpReceiver::Init(exports);
  node_webrtc::RTCRtpSender::Init(exports);
  node_webrtc::RTCRtpTransceiver::Init(exports);
  node_webrtc::LegacyStatsReport::Init(exports);
  node_webrtc::RTCStatsResponse::Init(exports);
  node_webrtc::RTCVideoSink::Init(exports);
  node_webrtc::RTCVideoSource::Init(exports);
  node::AtExit(dispose);
}

NODE_MODULE(wrtc, init)
