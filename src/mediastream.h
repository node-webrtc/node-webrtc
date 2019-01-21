/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_MEDIASTREAM_H_
#define SRC_MEDIASTREAM_H_

#include <memory>

#include <nan.h>
#include <webrtc/rtc_base/scoped_ref_ptr.h>
#include <v8.h>  // IWYU pragma: keep

#include "src/wrap.h"

namespace webrtc {

class MediaStreamInterface;
class MediaStreamTrackInterface;  // IWYU pragma: keep

}  // namespace webrc

namespace node_webrtc {

class MediaStreamTrack;  // IWYU pragma: keep
class PeerConnectionFactory;

class MediaStream
  : public Nan::ObjectWrap {
 public:
  ~MediaStream() override;

  static void Init(v8::Handle<v8::Object> exports);

  static ::node_webrtc::Wrap <
  MediaStream*,
  rtc::scoped_refptr<webrtc::MediaStreamInterface>,
  std::shared_ptr<node_webrtc::PeerConnectionFactory>
  > * wrap();

  static Nan::Persistent<v8::FunctionTemplate>& tpl();

  rtc::scoped_refptr<webrtc::MediaStreamInterface> stream() { return _stream; }

 private:
  explicit MediaStream(std::shared_ptr<node_webrtc::PeerConnectionFactory>&& factory = nullptr);

  explicit MediaStream(
      std::vector<node_webrtc::MediaStreamTrack*>&& tracks,
      std::shared_ptr<node_webrtc::PeerConnectionFactory>&& factory = nullptr);

  explicit MediaStream(
      rtc::scoped_refptr<webrtc::MediaStreamInterface>&& stream,
      std::shared_ptr<node_webrtc::PeerConnectionFactory>&& factory = nullptr);

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

  const std::shared_ptr<node_webrtc::PeerConnectionFactory> _factory;
  const rtc::scoped_refptr<webrtc::MediaStreamInterface> _stream;
  const bool _shouldReleaseFactory;
};


}  // namespace node_webrtc

#endif  // SRC_MEDIASTREAM_H_
