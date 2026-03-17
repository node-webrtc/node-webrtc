/*
 *  Copyright (c) 2019 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <memory>

#include <webrtc/api/array_view.h>
#include <webrtc/api/scoped_refptr.h>
#include <webrtc/modules/audio_device/include/audio_device.h>
#include <webrtc/modules/audio_device/include/test_audio_device.h>
#include <webrtc/rtc_base/buffer.h>
#include <webrtc/rtc_base/event.h>
#include <webrtc/rtc_base/ref_counted_object.h>

namespace webrtc {
class AudioTransport;
}

namespace node_webrtc {

// TestAudioDeviceModule implements an AudioDevice module that can act both as a
// capturer and a renderer. It will use 10ms audio frames.
class TestAudioDeviceModule
    : public rtc::RefCountedObject<webrtc::AudioDeviceModule> {
public:
  TestAudioDeviceModule() = default;
  TestAudioDeviceModule(const TestAudioDeviceModule &) = delete;
  TestAudioDeviceModule(TestAudioDeviceModule &&) = delete;
  TestAudioDeviceModule &operator=(const TestAudioDeviceModule &) = delete;
  TestAudioDeviceModule &operator=(TestAudioDeviceModule &&) = delete;
  ~TestAudioDeviceModule() override = default;

  // Returns the number of samples that Capturers and Renderers with this
  // sampling frequency will work with every time Capture or Render is called.
  static size_t SamplesPerFrame(int sampling_frequency_in_hz);

  // Creates a new TestAudioDeviceModule. When capturing or playing, 10 ms audio
  // frames will be processed every 10ms / |speed|.
  // |capturer| is an object that produces audio data. Can be nullptr if this
  // device is never used for recording.
  // |renderer| is an object that receives audio data that would have been
  // played out. Can be nullptr if this device is never used for playing.
  // Use one of the Create... functions to get these instances.
  static rtc::scoped_refptr<TestAudioDeviceModule> CreateTestAudioDeviceModule(
      std::unique_ptr<webrtc::TestAudioDeviceModule::Capturer> capturer,
      std::unique_ptr<webrtc::TestAudioDeviceModule::Renderer> renderer,
      float speed = 1);

  static std::unique_ptr<webrtc::TestAudioDeviceModule::Capturer>
  CreateZeroCapturer(int sampling_frequency_in_hz, int num_channels);

  int32_t Init() override = 0;

  int32_t RegisterAudioCallback(webrtc::AudioTransport *callback) override = 0;

  int32_t StartPlayout() override = 0;

  int32_t StopPlayout() override = 0;

  int32_t StartRecording() override = 0;

  int32_t StopRecording() override = 0;

  bool Playing() const override = 0;

  bool Recording() const override = 0;

  // Blocks until the Renderer refuses to receive data.
  // Returns false if |timeout_ms| passes before that happens.
  virtual bool
  WaitForPlayoutEnd(webrtc::TimeDelta timeout_ms = rtc::Event::kForever) = 0;
  // Blocks until the Recorder stops producing data.
  // Returns false if |timeout_ms| passes before that happens.
  virtual bool
  WaitForRecordingEnd(webrtc::TimeDelta timeout_ms = rtc::Event::kForever) = 0;
};

} // namespace node_webrtc
