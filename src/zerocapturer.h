/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_ZERO_CAPTURER_H_
#define SRC_ZERO_CAPTURER_H_

#include <memory>

#include "src/webrtc/fake_audio_device.h"

namespace node_webrtc {

class ZeroCapturer: public node_webrtc::FakeAudioDevice::Capturer {
 public:
  ZeroCapturer(int sampling_frequency_in_hz): _sampling_frequency_in_hz(sampling_frequency_in_hz) {}

  virtual int SamplingFrequency() const {
    return _sampling_frequency_in_hz;
  }

  virtual bool Capture(rtc::BufferT<int16_t>*) {
    return false;
  }

  static std::unique_ptr<ZeroCapturer> Create(int sampling_frequency_in_hz) {
    return std::unique_ptr<ZeroCapturer>(new ZeroCapturer(sampling_frequency_in_hz));
  }

 private:
  int _sampling_frequency_in_hz;
};

}  // namespace node_webrtc

#endif  // SRC_ZERO_CAPTURER_H_
