/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_MEDIASTREAM_H_
#define SRC_MEDIASTREAM_H_

#include "nan.h"
#include "v8.h"

#include "webrtc/api/mediastream.h"

#include "src/bidimap.h"
#include "src/mediastreamtrack.h"
#include "src/peerconnectionfactory.h"

namespace node_webrtc {

class MediaStream
  : public Nan::ObjectWrap {
 public:
  explicit MediaStream(std::shared_ptr<node_webrtc::PeerConnectionFactory>&& factory = nullptr);

  explicit MediaStream(
      std::vector<node_webrtc::MediaStreamTrack*>&& tracks,
      std::shared_ptr<node_webrtc::PeerConnectionFactory>&& factory = nullptr);

  explicit MediaStream(
      rtc::scoped_refptr<webrtc::MediaStreamInterface>&& stream,
      std::shared_ptr<node_webrtc::PeerConnectionFactory>&& factory = nullptr);

  ~MediaStream() override;

  static void Init(v8::Handle<v8::Object> exports);
  static Nan::Persistent<v8::Function> constructor;
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

  static MediaStream* GetOrCreate(
      std::shared_ptr<PeerConnectionFactory>,
      rtc::scoped_refptr<webrtc::MediaStreamInterface>);
  static void Release(MediaStream*);

  rtc::scoped_refptr<webrtc::MediaStreamInterface> stream() { return _stream; }

 private:
  std::vector<rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>> tracks() {
    auto tracks = std::vector<rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>>();
    for (auto const& track : _stream->GetAudioTracks()) {
      tracks.emplace_back(track);
    }
    for (auto const& track : _stream->GetVideoTracks()) {
      tracks.emplace_back(track);
    }
    return tracks;
  }

  const std::shared_ptr<node_webrtc::PeerConnectionFactory> _factory;
  const rtc::scoped_refptr<webrtc::MediaStreamInterface> _stream;
  const bool _shouldReleaseFactory;

  static BidiMap<rtc::scoped_refptr<webrtc::MediaStreamInterface>, MediaStream*> _streams;
};

}  // namespace node_webrtc

#endif  // SRC_MEDIASTREAM_H_
