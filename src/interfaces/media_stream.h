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
#include <webrtc/api/scoped_refptr.h>

#include "src/converters/napi.h"
#include "src/node/wrap.h"

namespace webrtc { class MediaStreamInterface; }
namespace webrtc { class MediaStreamTrackInterface; }

namespace node_webrtc {

class MediaStreamTrack;
class PeerConnectionFactory;
struct RTCMediaStreamInit;

class MediaStream
  : public Napi::ObjectWrap<MediaStream> {
 public:
  MediaStream(const Napi::CallbackInfo&);

  static void Init(Napi::Env, Napi::Object);

  static Napi::FunctionReference& constructor();

  static ::node_webrtc::Wrap <
  MediaStream*,
  rtc::scoped_refptr<webrtc::MediaStreamInterface>,
  PeerConnectionFactory*
  > * wrap();

  rtc::scoped_refptr<webrtc::MediaStreamInterface> stream();

 private:
  class Impl {
   public:
    Impl& operator=(Impl&& other) noexcept {
      if (&other != this) {
        _factory = other._factory;
        other._factory = nullptr;
        _stream = std::move(other._stream);
        _shouldReleaseFactory = other._shouldReleaseFactory;
        if (_shouldReleaseFactory) {
          other._shouldReleaseFactory = false;
        }
      }
      return *this;
    }

    ~Impl();

    explicit Impl(PeerConnectionFactory* factory = nullptr);

    Impl(std::vector<MediaStreamTrack*>&& tracks, PeerConnectionFactory* factory = nullptr);

    Impl(rtc::scoped_refptr<webrtc::MediaStreamInterface>&& stream, PeerConnectionFactory* factory = nullptr);

    Impl(const RTCMediaStreamInit& init, PeerConnectionFactory* factory = nullptr);

    PeerConnectionFactory* _factory;
    rtc::scoped_refptr<webrtc::MediaStreamInterface> _stream;
    bool _shouldReleaseFactory;
  };

  static MediaStream* Create(
      PeerConnectionFactory*,
      rtc::scoped_refptr<webrtc::MediaStreamInterface>);

  Napi::Value GetId(const Napi::CallbackInfo&);
  Napi::Value GetActive(const Napi::CallbackInfo&);

  Napi::Value GetAudioTracks(const Napi::CallbackInfo&);
  Napi::Value GetVideoTracks(const Napi::CallbackInfo&);
  Napi::Value GetTracks(const Napi::CallbackInfo&);
  Napi::Value GetTrackById(const Napi::CallbackInfo&);
  Napi::Value AddTrack(const Napi::CallbackInfo&);
  Napi::Value RemoveTrack(const Napi::CallbackInfo&);
  Napi::Value Clone(const Napi::CallbackInfo&);

  std::vector<rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>> tracks();

  Impl _impl;
};

DECLARE_TO_AND_FROM_NAPI(MediaStream*)

}  // namespace node_webrtc
