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

namespace node_webrtc {

class PeerConnectionFactory;

template <typename K, typename V> class BidiMap;

class MediaStreamTrack
  : public node_webrtc::AsyncObjectWrapWithLoop<MediaStreamTrack>
  , public webrtc::ObserverInterface {
 public:
  MediaStreamTrack(
      std::shared_ptr<node_webrtc::PeerConnectionFactory>&& factory,
      rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>&& track);

  ~MediaStreamTrack() override;

  static void Init(v8::Handle<v8::Object> exports);
  static Nan::Persistent<v8::Function> constructor;
  static NAN_METHOD(New);

  static NAN_GETTER(GetEnabled);
  static NAN_GETTER(GetId);
  static NAN_GETTER(GetKind);
  static NAN_GETTER(GetReadyState);

  static NAN_METHOD(Clone);
  static NAN_METHOD(JsStop);

  // ObserverInterface
  void OnChanged() override;

  rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track() { return _track; }

  static MediaStreamTrack* GetOrCreate(
      std::shared_ptr<PeerConnectionFactory>,
      rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>);
  static void Release(MediaStreamTrack*);

  std::shared_ptr<node_webrtc::PeerConnectionFactory> factory() { return _factory; }

  void OnPeerConnectionClosed();

 private:
  const std::shared_ptr<node_webrtc::PeerConnectionFactory> _factory;
  const rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> _track;

  static BidiMap<rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>, MediaStreamTrack*> _tracks;
};

}  // namespace node_webrtc

#endif  // SRC_MEDIASTREAMTRACK_H_
