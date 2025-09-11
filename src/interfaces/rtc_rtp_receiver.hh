/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <memory>

#include <node-addon-api/napi.h>
#include <webrtc/api/scoped_refptr.h>

#include "src/converters/napi.hh"
#include "src/interfaces/media_stream_track.hh"
#include "src/interfaces/rtc_dtls_transport.hh"
#include "src/node/async_object_wrap.hh"
#include "src/node/ref_ptr.hh"
#include "src/node/wrap.hh"

namespace webrtc {
class RtpReceiverInterface;
}

namespace node_webrtc {

class PeerConnectionFactory;

class RTCRtpReceiver : public AsyncObjectWrap<RTCRtpReceiver> {
public:
  explicit RTCRtpReceiver(const Napi::CallbackInfo &);
  ~RTCRtpReceiver() override;
  RTCRtpReceiver(const RTCRtpReceiver &) = delete;
  RTCRtpReceiver(RTCRtpReceiver &&) = delete;
  RTCRtpReceiver &operator=(const RTCRtpReceiver &) = delete;
  RTCRtpReceiver &operator=(RTCRtpReceiver &&) = delete;

  static void Init(Napi::Env, Napi::Object);

  static ::node_webrtc::Wrap<RTCRtpReceiver *,
                             rtc::scoped_refptr<webrtc::RtpReceiverInterface>,
                             PeerConnectionFactory *> *
  wrap();

  static Napi::FunctionReference &constructor();

  rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver() {
    return _receiver;
  }

private:
  static RTCRtpReceiver *
  Create(PeerConnectionFactory *,
         rtc::scoped_refptr<webrtc::RtpReceiverInterface>);

  Napi::Value GetTrack(const Napi::CallbackInfo &);
  Napi::Value GetTransport(const Napi::CallbackInfo &);
  Napi::Value GetRtcpTransport(const Napi::CallbackInfo &);

  static Napi::Value GetCapabilities(const Napi::CallbackInfo &);

  Napi::Value GetParameters(const Napi::CallbackInfo &);
  Napi::Value GetContributingSources(const Napi::CallbackInfo &);
  Napi::Value GetSynchronizationSources(const Napi::CallbackInfo &);
  Napi::Value GetStats(const Napi::CallbackInfo &);

  RefPtr<PeerConnectionFactory> _factory;
  rtc::scoped_refptr<webrtc::RtpReceiverInterface> _receiver;
  OwnedWrap<MediaStreamTrack> _track_wrap;
  OwnedWrap<RTCDtlsTransport> _transport_wrap;
};

DECLARE_TO_AND_FROM_NAPI(RTCRtpReceiver *)

} // namespace node_webrtc
