/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include <node.h>
#include <v8.h>

#include "src/datachannel.h"
#include "src/errorfactory.h"
#include "src/getusermedia.h"
#include "src/i420helpers.h"
#include "src/legacyrtcstatsreport.h"
#include "src/mediastream.h"
#include "src/mediastreamtrack.h"
#include "src/peerconnection.h"
#include "src/peerconnectionfactory.h"
#include "src/rtcaudiosink.h"
#include "src/rtcaudiosource.h"
#include "src/rtcdtlstransport.h"
#include "src/rtcrtpreceiver.h"
#include "src/rtcrtpsender.h"
#include "src/rtcrtptransceiver.h"
#include "src/rtcstatsresponse.h"
#include "src/rtcvideosink.h"
#include "src/rtcvideosource.h"

#ifdef DEBUG
#include "src/test.h"
#endif

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
  node_webrtc::RTCAudioSink::Init(exports);
  node_webrtc::RTCAudioSource::Init(exports);
  node_webrtc::RTCDtlsTransport::Init(exports);
  node_webrtc::RTCRtpReceiver::Init(exports);
  node_webrtc::RTCRtpSender::Init(exports);
  node_webrtc::RTCRtpTransceiver::Init(exports);
  node_webrtc::LegacyStatsReport::Init(exports);
  node_webrtc::RTCStatsResponse::Init(exports);
  node_webrtc::RTCVideoSink::Init(exports);
  node_webrtc::RTCVideoSource::Init(exports);
#ifdef DEBUG
  node_webrtc::Test::Init(exports);
#endif
  node::AtExit(dispose);
}

NODE_MODULE(wrtc, init)
