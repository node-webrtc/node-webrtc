/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include <cassert>
#include <node-addon-api/napi.h>
#include <uv.h>

#include "src/interfaces/media_stream.hh"
#include "src/interfaces/media_stream_track.hh"
#include "src/interfaces/rtc_audio_sink.hh"
#include "src/interfaces/rtc_audio_source.hh"
#include "src/interfaces/rtc_data_channel.hh"
#include "src/interfaces/rtc_dtls_transport.hh"
#include "src/interfaces/rtc_ice_transport.hh"
#include "src/interfaces/rtc_peer_connection.hh"
#include "src/interfaces/rtc_peer_connection/peer_connection_factory.hh"
#include "src/interfaces/rtc_rtp_receiver.hh"
#include "src/interfaces/rtc_rtp_sender.hh"
#include "src/interfaces/rtc_rtp_transceiver.hh"
#include "src/interfaces/rtc_sctp_transport.hh"
#include "src/interfaces/rtc_video_sink.hh"
#include "src/interfaces/rtc_video_source.hh"
#include "src/methods/get_display_media.hh"
#include "src/methods/get_user_media.hh"
#include "src/methods/i420_helpers.hh"
#include "src/node/async_context_releaser.hh"
#include "src/node/error_factory.hh"

#ifdef DEBUG
#include "src/test.hh"
#endif

static void dispose(void *) { node_webrtc::PeerConnectionFactory::Dispose(); }

static Napi::Object Init(Napi::Env env, Napi::Object exports) {
  node_webrtc::ErrorFactory::Init(env, exports);
  node_webrtc::GetDisplayMedia::Init(env, exports);
  node_webrtc::GetUserMedia::Init(env, exports);
  node_webrtc::I420Helpers::Init(env, exports);
  node_webrtc::MediaStream::Init(env, exports);
  node_webrtc::MediaStreamTrack::Init(env, exports);
  node_webrtc::PeerConnectionFactory::Init(env, exports);
  node_webrtc::RTCAudioSink::Init(env, exports);
  node_webrtc::RTCAudioSource::Init(env, exports);
  node_webrtc::RTCDataChannel::Init(env, exports);
  node_webrtc::RTCIceTransport::Init(env, exports);
  node_webrtc::RTCDtlsTransport::Init(env, exports);
  node_webrtc::RTCPeerConnection::Init(env, exports);
  node_webrtc::RTCRtpReceiver::Init(env, exports);
  node_webrtc::RTCRtpSender::Init(env, exports);
  node_webrtc::RTCRtpTransceiver::Init(env, exports);
  node_webrtc::RTCSctpTransport::Init(env, exports);
  node_webrtc::RTCVideoSink::Init(env, exports);
  node_webrtc::RTCVideoSource::Init(env, exports);
#if DEBUG && defined(__x86_64__)
  node_webrtc::Test::Init(env, exports);
#endif

  auto status = napi_add_env_cleanup_hook(env, dispose, nullptr);
  assert(status == napi_ok);
  (void)status; // Ignore unused variable warning in release builds

  return exports;
}

NODE_API_MODULE(wrtc_napi, Init)
