/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <atomic>
#include <memory>

#include <node-addon-api/napi.h>
#include <webrtc/api/media_stream_interface.h>
#include <webrtc/api/scoped_refptr.h>
#include <webrtc/pc/local_audio_source.h>

#include "src/dictionaries/node_webrtc/rtc_on_data_event_dict.h"
#include "src/interfaces/rtc_peer_connection/peer_connection_factory.h"

namespace node_webrtc {

class RTCAudioTrackSource : public webrtc::LocalAudioSource {
 public:
  RTCAudioTrackSource() {}

  ~RTCAudioTrackSource() override {
    PeerConnectionFactory::Release();
    _factory = nullptr;
  }

  SourceState state() const override {
    return webrtc::MediaSourceInterface::SourceState::kLive;
  }

  bool remote() const override {
    return false;
  }

  void PushData(RTCOnDataEventDict dict) {
    webrtc::AudioTrackSinkInterface* sink = _sink;
    if (sink && dict.numberOfFrames.IsJust()) {
      sink->OnData(
          dict.samples,
          dict.bitsPerSample,
          dict.sampleRate,
          dict.channelCount,
          dict.numberOfFrames.UnsafeFromJust()
      );
    }
    // HACK(mroberts): I'd rather we use a smart pointer.
    delete[] dict.samples;
  }

  void AddSink(webrtc::AudioTrackSinkInterface* sink) override {
    _sink = sink;
  }

  void RemoveSink(webrtc::AudioTrackSinkInterface*) override {
    _sink = nullptr;
  }

 private:
  PeerConnectionFactory* _factory = PeerConnectionFactory::GetOrCreateDefault();

  std::atomic<webrtc::AudioTrackSinkInterface*> _sink = {nullptr};
};

class RTCAudioSource
  : public Napi::ObjectWrap<RTCAudioSource> {
 public:
  RTCAudioSource(const Napi::CallbackInfo&);

  static void Init(Napi::Env, Napi::Object);

 private:
  static Napi::FunctionReference& constructor();

  Napi::Value CreateTrack(const Napi::CallbackInfo&);
  Napi::Value OnData(const Napi::CallbackInfo&);

  rtc::scoped_refptr<RTCAudioTrackSource> _source;
};

}  // namespace node_webrtc
