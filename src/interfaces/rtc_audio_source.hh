/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <mutex>
#include <shared_mutex>
#include <vector>

#include <node-addon-api/napi.h>
#include <webrtc/api/media_stream_interface.h>
#include <webrtc/api/scoped_refptr.h>
#include <webrtc/pc/local_audio_source.h>

#include "src/dictionaries/node_webrtc/rtc_on_data_event_dict.hh"
#include "src/interfaces/media_stream_track.hh"
#include "src/interfaces/rtc_peer_connection/peer_connection_factory.hh"
#include "src/node/wrap.hh"

namespace node_webrtc {

class RTCAudioTrackSource : public webrtc::LocalAudioSource {
public:
  RTCAudioTrackSource(const RTCAudioTrackSource &) = delete;
  RTCAudioTrackSource(RTCAudioTrackSource &&) = delete;
  RTCAudioTrackSource &operator=(const RTCAudioTrackSource &) = delete;
  RTCAudioTrackSource &operator=(RTCAudioTrackSource &&) = delete;
  RTCAudioTrackSource() = default;
  ~RTCAudioTrackSource() override {
    PeerConnectionFactory::Release();
    _factory = nullptr;
    // No need to acquire mutex, since this MUST only be destroyed when there
    // are no other functions being called on it
    _sinks.clear();
  }

  [[nodiscard]] SourceState state() const override {
    return webrtc::MediaSourceInterface::SourceState::kLive;
  }

  [[nodiscard]] bool remote() const override { return false; }

  void PushData(RTCOnDataEventDict dict) {
    if (dict.numberOfFrames.IsJust()) {
      std::shared_lock<std::shared_mutex> lock{_sinks_mutex};
      for (auto sink : _sinks) {
        sink->OnData(dict.samples, dict.bitsPerSample, dict.sampleRate,
                     dict.channelCount, dict.numberOfFrames.UnsafeFromJust());
      }
    }

    // HACK(mroberts): I'd rather we use a smart pointer.
    delete[] dict.samples;
  }

  void AddSink(webrtc::AudioTrackSinkInterface *sink) override {
    std::unique_lock<std::shared_mutex> lock{_sinks_mutex};
    _sinks.push_back(sink);
  }

  void RemoveSink(webrtc::AudioTrackSinkInterface *sink) override {
    std::unique_lock<std::shared_mutex> lock{_sinks_mutex};
    auto it = std::find(_sinks.begin(), _sinks.end(), sink);

    if (it != _sinks.end()) {
      _sinks.erase(it);
    }
  }

private:
  PeerConnectionFactory *_factory = PeerConnectionFactory::GetOrCreateDefault();

  std::shared_mutex _sinks_mutex;
  std::vector<webrtc::AudioTrackSinkInterface *> _sinks;
};

class RTCAudioSource : public Napi::ObjectWrap<RTCAudioSource> {
public:
  RTCAudioSource(const Napi::CallbackInfo &);

  static void Init(Napi::Env, Napi::Object);

private:
  static Napi::FunctionReference &constructor();

  Napi::Value CreateTrack(const Napi::CallbackInfo &);
  Napi::Value OnData(const Napi::CallbackInfo &);

  rtc::scoped_refptr<RTCAudioTrackSource> _source;
  OwnedWrap<MediaStreamTrack> _track_wrap;
};

} // namespace node_webrtc
