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
#include <webrtc/api/scoped_refptr.h>
#include <v8.h>

#include "src/wrap.h"

namespace webrtc {

class MediaStreamInterface;
class MediaStreamTrackInterface;

}  // namespace webrc

namespace node_webrtc {

class MediaStreamTrack;
class PeerConnectionFactory;

class MediaStream
  : public Nan::ObjectWrap {
 public:
  ~MediaStream() override;

  static void Init(v8::Handle<v8::Object> exports);

  static ::node_webrtc::Wrap <
  MediaStream*,
  rtc::scoped_refptr<webrtc::MediaStreamInterface>,
  std::shared_ptr<PeerConnectionFactory>
  > * wrap();

  static Nan::Persistent<v8::FunctionTemplate>& tpl();

  rtc::scoped_refptr<webrtc::MediaStreamInterface> stream() { return _stream; }

 private:
  explicit MediaStream(std::shared_ptr<PeerConnectionFactory>&& factory = nullptr);

  explicit MediaStream(
      std::vector<MediaStreamTrack*>&& tracks,
      std::shared_ptr<PeerConnectionFactory>&& factory = nullptr);

  explicit MediaStream(
      rtc::scoped_refptr<webrtc::MediaStreamInterface>&& stream,
      std::shared_ptr<PeerConnectionFactory>&& factory = nullptr);

  static MediaStream* Create(
      std::shared_ptr<PeerConnectionFactory>,
      rtc::scoped_refptr<webrtc::MediaStreamInterface>);

  static Nan::Persistent<v8::Function>& constructor();

  static NAN_METHOD(New);

  static NAN_GETTER(GetId);
  static NAN_GETTER(GetActive);

  static NAN_METHOD(GetAudioTracks);
  static NAN_METHOD(GetVideoTracks);
  static NAN_METHOD(GetTracks);
  static NAN_METHOD(GetTrackById);
  static NAN_METHOD(AddTrack);
  static NAN_METHOD(RemoveTrack);
  static NAN_METHOD(Clone);

  std::vector<rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>> tracks();

  const std::shared_ptr<PeerConnectionFactory> _factory;
  const rtc::scoped_refptr<webrtc::MediaStreamInterface> _stream;
  const bool _shouldReleaseFactory;
};

}  // namespace node_webrtc
