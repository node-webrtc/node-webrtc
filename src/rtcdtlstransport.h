/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_RTCDTLSTRANSPORT_H_
#define SRC_RTCDTLSTRANSPORT_H_

#include <mutex>

#include <nan.h>
#include <webrtc/api/dtls_transport_interface.h>
#include <webrtc/rtc_base/scoped_ref_ptr.h>
#include <v8.h>  // IWYU pragma: keep

#include "src/asyncobjectwrapwithloop.h"  // IWYU pragma: keep
#include "src/wrap.h"

namespace node_webrtc {

class PeerConnectionFactory;

class RTCDtlsTransport
  : public AsyncObjectWrapWithLoop<RTCDtlsTransport>
  , public webrtc::DtlsTransportObserverInterface {
 public:
  ~RTCDtlsTransport() override = default;

  static void Init(v8::Handle<v8::Object> exports);

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
  explicit RTCDtlsTransport(
      std::shared_ptr<PeerConnectionFactory>,
      rtc::scoped_refptr<webrtc::DtlsTransportInterface>);

  static Nan::Persistent<v8::FunctionTemplate>& tpl();

  static NAN_METHOD(New);

  static NAN_GETTER(GetState);

  static RTCDtlsTransport* Create(
      std::shared_ptr<PeerConnectionFactory>,
      rtc::scoped_refptr<webrtc::DtlsTransportInterface>);

  static Nan::Persistent<v8::Function>& constructor();

  std::mutex _mutex;
  webrtc::DtlsTransportState _state;

  const std::shared_ptr<PeerConnectionFactory> _factory;
  const rtc::scoped_refptr<webrtc::DtlsTransportInterface> _transport;
};

}  // namespace node_webrtc

#endif  // SRC_RTCDTLSTRANSPORT_H_
