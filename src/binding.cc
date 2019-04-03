/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include <node-addon-api/napi.h>
#include <node.h>
#include <v8.h>

#include "src/converters/napi.h"
#include "src/interfaces/legacy_rtc_stats_report.h"
#include "src/interfaces/media_stream.h"
#include "src/interfaces/media_stream_track.h"
#include "src/interfaces/rtc_audio_sink.h"
#include "src/interfaces/rtc_audio_source.h"
#include "src/interfaces/rtc_data_channel.h"
#include "src/interfaces/rtc_dtls_transport.h"
#include "src/interfaces/rtc_peer_connection.h"
#include "src/interfaces/rtc_peer_connection/peer_connection_factory.h"
#include "src/interfaces/rtc_rtp_receiver.h"
#include "src/interfaces/rtc_rtp_sender.h"
#include "src/interfaces/rtc_rtp_transceiver.h"
#include "src/interfaces/rtc_stats_response.h"
#include "src/interfaces/rtc_video_sink.h"
#include "src/interfaces/rtc_video_source.h"
#include "src/methods/get_user_media.h"
#include "src/methods/i420_helpers.h"
#include "src/node/error_factory.h"

#ifdef DEBUG
#include "src/test.h"
#endif

static void dispose(void*) {
  node_webrtc::PeerConnectionFactory::Dispose();
}

static void init(Napi::Env env, v8::Handle<v8::Object> exports, v8::Handle<v8::Object>) {
  (void) env;
  node_webrtc::I420Helpers::Init(exports);
  node_webrtc::PeerConnectionFactory::Init(exports);
  node_webrtc::RTCPeerConnection::Init(exports);
  node_webrtc::RTCDataChannel::Init(exports);
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

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  node_webrtc::ErrorFactory::Init(env, exports);
  node_webrtc::GetUserMedia::Init(env, exports);

  auto v8_exports = node_webrtc::napi::UnsafeToV8(exports).As<v8::Object>();
  init(env, v8_exports, v8_exports);

  return exports;
}

NODE_API_MODULE(wrtc_napi, Init)
