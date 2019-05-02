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
#include <webrtc/api/ice_transport_interface.h>
#include <webrtc/api/scoped_refptr.h>
#include <webrtc/p2p/base/ice_transport_internal.h>
#include <webrtc/rtc_base/third_party/sigslot/sigslot.h>

#include "src/enums/node_webrtc/rtc_ice_component.h"
#include "src/node/async_object_wrap_with_loop.h"
#include "src/node/wrap.h"

namespace cricket { class IceTransportInternal; }

namespace node_webrtc {

class PeerConnectionFactory;

class RTCIceTransport
  : public AsyncObjectWrapWithLoop<RTCIceTransport>
  , public sigslot::has_slots<sigslot::multi_threaded_local> {
 public:
  explicit RTCIceTransport(const Napi::CallbackInfo&);

  ~RTCIceTransport() override;

  static void Init(Napi::Env, Napi::Object);

  static ::node_webrtc::Wrap <
  RTCIceTransport*,
  rtc::scoped_refptr<webrtc::IceTransportInterface>,
  PeerConnectionFactory*
  > * wrap();

  void OnRTCDtlsTransportStopped();

 protected:
  void Stop() override;

 private:
  static Napi::FunctionReference& constructor();

  static RTCIceTransport* Create(
      PeerConnectionFactory*,
      rtc::scoped_refptr<webrtc::IceTransportInterface>);

  void OnStateChanged(cricket::IceTransportInternal*);
  void OnGatheringStateChanged(cricket::IceTransportInternal*);

  void TakeSnapshot();

  Napi::Value GetRole(const Napi::CallbackInfo&);
  Napi::Value GetComponent(const Napi::CallbackInfo&);
  Napi::Value GetState(const Napi::CallbackInfo&);
  Napi::Value GetGatheringState(const Napi::CallbackInfo&);

  Napi::Value GetLocalCandidates(const Napi::CallbackInfo&);
  Napi::Value GetRemoteCandidates(const Napi::CallbackInfo&);
  Napi::Value GetSelectedCandidatePair(const Napi::CallbackInfo&);
  Napi::Value GetLocalParameters(const Napi::CallbackInfo&);
  Napi::Value GetRemoteParameters(const Napi::CallbackInfo&);

  RTCIceComponent _component = RTCIceComponent::kRtp;
  PeerConnectionFactory* _factory;
  cricket::IceGatheringState _gathering_state = cricket::IceGatheringState::kIceGatheringNew;
  std::mutex _mutex{};
  cricket::IceRole _role = cricket::IceRole::ICEROLE_UNKNOWN;
  webrtc::IceTransportState _state = webrtc::IceTransportState::kNew;
  rtc::scoped_refptr<webrtc::IceTransportInterface> _transport;
};

}  // namespace node_webrtc
