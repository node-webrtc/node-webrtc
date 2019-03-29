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

#include <nan.h>
#include <webrtc/api/media_stream_interface.h>
#include <webrtc/api/scoped_refptr.h>
#include <webrtc/pc/local_audio_source.h>
#include <v8.h>

#include "src/converters/dictionaries.h"
#include "src/interfaces/rtc_peer_connection/peer_connection_factory.h"

namespace node_webrtc {

class RTCAudioTrackSource : public webrtc::LocalAudioSource {
 public:
  RTCAudioTrackSource() = default;

  ~RTCAudioTrackSource() override = default;

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
  const std::shared_ptr<PeerConnectionFactory> _factory = PeerConnectionFactory::GetOrCreateDefault();

  std::atomic<webrtc::AudioTrackSinkInterface*> _sink = {nullptr};
};

class RTCAudioSource
  : public Nan::ObjectWrap {
 public:
  RTCAudioSource();

  ~RTCAudioSource() override = default;

  //
  // Nodejs wrapping.
  //
  static void Init(v8::Handle<v8::Object> exports);

 private:
  static Nan::Persistent<v8::Function>& constructor();

  static NAN_METHOD(New);

  static NAN_METHOD(CreateTrack);
  static NAN_METHOD(OnData);

  rtc::scoped_refptr<RTCAudioTrackSource> _source;
};

}  // namespace node_webrtc
