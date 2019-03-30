/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <memory>

#include <nan.h>
#include <webrtc/api/media_stream_interface.h>
#include <webrtc/api/scoped_refptr.h>
#include <v8.h>

#include "src/converters.h"
#include "src/converters/v8.h"
#include "src/node/async_object_wrap_with_loop.h"
#include "src/node/wrap.h"

namespace node_webrtc {

class PeerConnectionFactory;

class MediaStreamTrack
  : public AsyncObjectWrapWithLoop<MediaStreamTrack>
  , public webrtc::ObserverInterface {
 public:
  ~MediaStreamTrack() override;

  static void Init(v8::Handle<v8::Object> exports);

  // ObserverInterface
  void OnChanged() override;

  void OnPeerConnectionClosed();

  bool active() { return _ended ? false : _track->state() == webrtc::MediaStreamTrackInterface::TrackState::kLive; }
  std::shared_ptr<PeerConnectionFactory> factory() { return _factory; }
  rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track() { return _track; }

  static ::node_webrtc::Wrap <
  MediaStreamTrack*,
  rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>,
  std::shared_ptr<PeerConnectionFactory>
  > * wrap();

  static Nan::Persistent<v8::FunctionTemplate>& tpl();

 protected:
  void Stop() override;

 private:
  MediaStreamTrack(
      std::shared_ptr<PeerConnectionFactory>&& factory,
      rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>&& track);

  static MediaStreamTrack* Create(
      std::shared_ptr<PeerConnectionFactory>,
      rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>);

  static Nan::Persistent<v8::Function>& constructor();

  static NAN_METHOD(New);

  static NAN_GETTER(GetEnabled);
  static NAN_SETTER(SetEnabled);
  static NAN_GETTER(GetId);
  static NAN_GETTER(GetKind);
  static NAN_GETTER(GetReadyState);
  static NAN_GETTER(GetMuted);

  static NAN_METHOD(Clone);
  static NAN_METHOD(JsStop);

  bool _ended = false;
  bool _enabled;
  const std::shared_ptr<PeerConnectionFactory> _factory;
  const rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> _track;
};

DECLARE_CONVERTER(MediaStreamTrack*, rtc::scoped_refptr<webrtc::AudioTrackInterface>)
DECLARE_CONVERTER(MediaStreamTrack*, rtc::scoped_refptr<webrtc::VideoTrackInterface>)
DECLARE_FROM_JS(rtc::scoped_refptr<webrtc::AudioTrackInterface>)
DECLARE_FROM_JS(rtc::scoped_refptr<webrtc::VideoTrackInterface>)
DECLARE_TO_AND_FROM_JS(MediaStreamTrack*)

}  // namespace node_webrtc
