/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <memory>

#include "src/webrtc/test_audio_device_module.h"

namespace node_webrtc {

class ZeroCapturer: public TestAudioDeviceModule::Capturer {
 public:
  explicit ZeroCapturer(int sampling_frequency_in_hz): _sampling_frequency_in_hz(sampling_frequency_in_hz) {}

  int SamplingFrequency() const override {
    return _sampling_frequency_in_hz;
  }

  bool Capture(rtc::BufferT<int16_t>* buffer) override {
    // NOTE(mroberts): If we don't fill this buffer once we trigger an assert.
    if (!_produced_output) {
      buffer->SetSize(TestAudioDeviceModule::SamplesPerFrame(_sampling_frequency_in_hz));
      _produced_output = true;
    }
    return false;
  }

  int NumChannels() const override {
    return 1;
  }

  static std::unique_ptr<ZeroCapturer> Create(int sampling_frequency_in_hz) {
    return std::make_unique<ZeroCapturer>(sampling_frequency_in_hz);
  }

 private:
  int _sampling_frequency_in_hz;
  bool _produced_output = false;
};

}  // namespace node_webrtc
