/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <memory>
#include <mutex>

#include <node-addon-api/napi.h>
#include <webrtc/api/dtls_transport_interface.h>
#include <webrtc/api/scoped_refptr.h>

#include "src/node/napi_async_object_wrap_with_loop.h"
#include "src/node/wrap.h"

namespace webrtc { class RTCError; }

namespace node_webrtc {

class PeerConnectionFactory;

class RTCDtlsTransport
  : public napi::AsyncObjectWrapWithLoop<RTCDtlsTransport>
  , public webrtc::DtlsTransportObserverInterface {
 public:
  explicit RTCDtlsTransport(const Napi::CallbackInfo&);

  static void Init(Napi::Env, Napi::Object);

  static ::node_webrtc::Wrap <
  RTCDtlsTransport*,
  rtc::scoped_refptr<webrtc::DtlsTransportInterface>,
  std::shared_ptr<PeerConnectionFactory>
  > * wrap();

  void OnStateChange(webrtc::DtlsTransportInformation) override;

  void OnError(webrtc::RTCError) override;

 protected:
  void Stop() override;

 private:
  static Napi::FunctionReference& constructor();

  Napi::Value GetState(const Napi::CallbackInfo&);

  static RTCDtlsTransport* Create(
      std::shared_ptr<PeerConnectionFactory>,
      rtc::scoped_refptr<webrtc::DtlsTransportInterface>);

  std::mutex _mutex;
  webrtc::DtlsTransportState _state;

  std::shared_ptr<PeerConnectionFactory> _factory;
  rtc::scoped_refptr<webrtc::DtlsTransportInterface> _transport;
};

}  // namespace node_webrtc
