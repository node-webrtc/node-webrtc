/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <mutex>

#include <node-addon-api/napi.h>
#include <webrtc/api/dtls_transport_interface.h>
#include <webrtc/api/scoped_refptr.h>

#include "src/node/async_object_wrap_with_loop.h"
#include "src/node/wrap.h"

namespace webrtc { class RTCError; }

namespace node_webrtc {

class PeerConnectionFactory;

class RTCDtlsTransport
  : public AsyncObjectWrapWithLoop<RTCDtlsTransport>
  , public webrtc::DtlsTransportObserverInterface {
 public:
  explicit RTCDtlsTransport(const Napi::CallbackInfo&);

  ~RTCDtlsTransport() override;

  static void Init(Napi::Env, Napi::Object);

  static ::node_webrtc::Wrap <
  RTCDtlsTransport*,
  rtc::scoped_refptr<webrtc::DtlsTransportInterface>,
  PeerConnectionFactory*
  > * wrap();

  void OnStateChange(webrtc::DtlsTransportInformation) override;

  void OnError(webrtc::RTCError) override;

 protected:
  void Stop() override;

 private:
  static Napi::FunctionReference& constructor();

  Napi::Value GetIceTransport(const Napi::CallbackInfo&);
  Napi::Value GetState(const Napi::CallbackInfo&);
  Napi::Value GetRemoteCertificates(const Napi::CallbackInfo&);

  static RTCDtlsTransport* Create(
      PeerConnectionFactory*,
      rtc::scoped_refptr<webrtc::DtlsTransportInterface>);

  std::mutex _mutex;
  webrtc::DtlsTransportState _state;
  std::vector<rtc::Buffer> _certs;

  PeerConnectionFactory* _factory;
  rtc::scoped_refptr<webrtc::DtlsTransportInterface> _transport;
};

}  // namespace node_webrtc
