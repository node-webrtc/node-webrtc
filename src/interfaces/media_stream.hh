/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <memory>

#include <node-addon-api/napi.h>
#include <webrtc/api/media_stream_interface.h>
#include <webrtc/api/scoped_refptr.h>

#include "src/converters/napi.hh"
#include "src/interfaces/media_stream_track.hh"
#include "src/interfaces/rtc_peer_connection/peer_connection_factory.hh"
#include "src/node/async_object_wrap.hh"
#include "src/node/ref_ptr.hh"
#include "src/node/wrap.hh"

namespace node_webrtc {

struct RTCMediaStreamInit;

class MediaStream : public AsyncObjectWrap<MediaStream> {
public:
  explicit MediaStream(const Napi::CallbackInfo &);
  ~MediaStream() override;

  MediaStream(const MediaStream &) = delete;
  MediaStream(MediaStream &&) = delete;
  MediaStream &operator=(const MediaStream &) = delete;
  MediaStream &operator=(MediaStream &&) = delete;

  static void Init(Napi::Env, Napi::Object);

  static Napi::FunctionReference &constructor();

  static ::node_webrtc::Wrap<MediaStream *,
                             rtc::scoped_refptr<webrtc::MediaStreamInterface>,
                             PeerConnectionFactory *> *
  wrap();

  rtc::scoped_refptr<webrtc::MediaStreamInterface> stream();

private:
  class Impl {
  public:
    explicit Impl(PeerConnectionFactory *factory = nullptr);
    ~Impl();

    Impl(std::vector<MediaStreamTrack *> &&tracks,
         PeerConnectionFactory *factory = nullptr);

    Impl(rtc::scoped_refptr<webrtc::MediaStreamInterface> &&stream,
         PeerConnectionFactory *factory = nullptr);

    Impl(const RTCMediaStreamInit &init,
         PeerConnectionFactory *factory = nullptr);

    Impl(const Impl &) = delete;
    Impl &operator=(const Impl &) = delete;
    Impl(Impl &&) = delete;
    Impl &operator=(Impl &&other) noexcept {
      if (&other != this) {
        _factory = std::move(other._factory);
        _stream = std::move(other._stream);
        _shouldReleaseFactory = other._shouldReleaseFactory;
        other._shouldReleaseFactory = false;
      }
      return *this;
    }

    RefPtr<PeerConnectionFactory> _factory;
    rtc::scoped_refptr<webrtc::MediaStreamInterface> _stream;
    bool _shouldReleaseFactory;
  };

  static MediaStream *Create(PeerConnectionFactory *,
                             rtc::scoped_refptr<webrtc::MediaStreamInterface>);

  Napi::Value GetId(const Napi::CallbackInfo &);
  Napi::Value GetActive(const Napi::CallbackInfo &);

  Napi::Value GetAudioTracks(const Napi::CallbackInfo &);
  Napi::Value GetVideoTracks(const Napi::CallbackInfo &);
  Napi::Value GetTracks(const Napi::CallbackInfo &);
  Napi::Value GetTrackById(const Napi::CallbackInfo &);
  Napi::Value AddTrack(const Napi::CallbackInfo &);
  Napi::Value RemoveTrack(const Napi::CallbackInfo &);
  Napi::Value Clone(const Napi::CallbackInfo &);

  std::vector<rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>> tracks();

  Impl _impl;
  OwnedWrap<MediaStreamTrack> _track_wrap;
};

DECLARE_TO_AND_FROM_NAPI(MediaStream *)

} // namespace node_webrtc
