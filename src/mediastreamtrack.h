/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_MEDIASTREAMTRACK_H_
#define SRC_MEDIASTREAMTRACK_H_

#include <memory>

#include <nan.h>
#include <webrtc/api/mediastreaminterface.h>
#include <webrtc/rtc_base/scoped_ref_ptr.h>
#include <v8.h>  // IWYU pragma: keep

#include "src/asyncobjectwrapwithloop.h"  // IWYU pragma: keep
#include "src/wrap.h"

namespace node_webrtc {

class PeerConnectionFactory;

class MediaStreamTrack
  : public node_webrtc::AsyncObjectWrapWithLoop<MediaStreamTrack>
  , public webrtc::ObserverInterface {
 public:
  ~MediaStreamTrack() override;

  static void Init(v8::Handle<v8::Object> exports);

  // ObserverInterface
  void OnChanged() override;

  void OnPeerConnectionClosed();

  bool active() { return _ended ? false : _track->state() == webrtc::MediaStreamTrackInterface::TrackState::kLive; }
  std::shared_ptr<node_webrtc::PeerConnectionFactory> factory() { return _factory; }
  rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track() { return _track; }

  static ::node_webrtc::Wrap <
  MediaStreamTrack*,
  rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>,
  std::shared_ptr<PeerConnectionFactory>
  > * wrap();

 private:
  MediaStreamTrack(
      std::shared_ptr<node_webrtc::PeerConnectionFactory>&& factory,
      rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>&& track);

  static MediaStreamTrack* Create(
      std::shared_ptr<PeerConnectionFactory>,
      rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>);

  static Nan::Persistent<v8::Function>& constructor();

  static NAN_METHOD(New);

  static NAN_GETTER(GetEnabled);
  static NAN_GETTER(GetId);
  static NAN_GETTER(GetKind);
  static NAN_GETTER(GetReadyState);
  static NAN_GETTER(GetMuted);

  static NAN_METHOD(Clone);
  static NAN_METHOD(JsStop);

  bool _ended;
  const std::shared_ptr<node_webrtc::PeerConnectionFactory> _factory;
  const rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> _track;
};

}  // namespace node_webrtc

#endif  // SRC_MEDIASTREAMTRACK_H_
