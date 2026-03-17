/*
 *  Copyright (c) 2019 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include "src/webrtc/test_audio_device_module.hh"

#include <algorithm>
#include <cstdlib>
#include <iosfwd>
#include <memory>
#include <src/rtc_base/ref_counted_object.h>
#include <type_traits>
#include <vector>

#include <absl/memory/memory.h>
#include <webrtc/common_audio/wav_file.h>
#include <webrtc/modules/audio_device/include/audio_device_default.h>
#include <webrtc/modules/audio_device/include/audio_device_defines.h>
#include <webrtc/modules/audio_device/include/test_audio_device.h>
#include <webrtc/rtc_base/buffer.h>
#include <webrtc/rtc_base/checks.h>
#include <webrtc/rtc_base/deprecated/recursive_critical_section.h>
#include <webrtc/rtc_base/event.h>
#include <webrtc/rtc_base/logging.h>
#include <webrtc/rtc_base/numerics/safe_conversions.h>
#include <webrtc/rtc_base/platform_thread.h>
#include <webrtc/rtc_base/random.h>
#include <webrtc/rtc_base/ref_counted_object.h>
#include <webrtc/rtc_base/thread.h>
#include <webrtc/rtc_base/thread_annotations.h>
#include <webrtc/rtc_base/time_utils.h>

namespace node_webrtc {

namespace {

constexpr int kFrameLengthUs = 10000;
constexpr int kFramesPerSecond = rtc::kNumMicrosecsPerSec / kFrameLengthUs;

// TestAudioDeviceModule implements an AudioDevice module that can act both as a
// capturer and a renderer. It will use 10ms audio frames.
class
    TestAudioDeviceModuleImpl // NOLINT(cppcoreguidelines-special-member-functions)
    : public webrtc::webrtc_impl::AudioDeviceModuleDefault<
          TestAudioDeviceModule> {
public:
  // Creates a new TestAudioDeviceModule. When capturing or playing, 10 ms audio
  // frames will be processed every 10ms / |speed|.
  // |capturer| is an object that produces audio data. Can be nullptr if this
  // device is never used for recording.
  // |renderer| is an object that receives audio data that would have been
  // played out. Can be nullptr if this device is never used for playing.
  // Use one of the Create... functions to get these instances.
  TestAudioDeviceModuleImpl(
      std::unique_ptr<webrtc::TestAudioDeviceModule::Capturer> capturer,
      std::unique_ptr<webrtc::TestAudioDeviceModule::Renderer> renderer,
      float speed = 1)
      : capturer_(std::move(capturer)), renderer_(std::move(renderer)),
        process_interval_us_(static_cast<int64_t>(kFrameLengthUs / speed)),
        done_rendering_(true, true), done_capturing_(true, true) {
    auto good_sample_rate = [](auto sr) {
      return sr == 8000 || sr == 16000 || sr == 32000 || sr == 44100 ||
             sr == 48000;
    };

    if (renderer_) {
      const int sample_rate = renderer_->SamplingFrequency();
      playout_buffer_.resize(
          SamplesPerFrame(sample_rate) * renderer_->NumChannels(), 0);
      RTC_CHECK(good_sample_rate(sample_rate));
    }
    if (capturer_) {
      RTC_CHECK(good_sample_rate(capturer_->SamplingFrequency()));
    }
  }

  ~TestAudioDeviceModuleImpl() override {
    // TODO(jack): figure out a way to not call these virtual methods during
    // the destructor.
    StopPlayout();   // NOLINT
    StopRecording(); // NOLINT
    if (thread_) {
      {
        rtc::CritScope cs(&lock_);
        stop_thread_ = true;
      }
      thread_->Finalize();
    }
  }

  int32_t Init() override {
    thread_ = absl::make_unique<rtc::PlatformThread>(
        rtc::PlatformThread::SpawnJoinable(
            [this]() { TestAudioDeviceModuleImpl::Run(this); },
            "TestAudioDeviceModuleImpl",
            rtc::ThreadAttributes{rtc::ThreadPriority::kHigh}));
    return 0;
  }

  int32_t RegisterAudioCallback(webrtc::AudioTransport *callback) override {
    rtc::CritScope cs(&lock_);
    RTC_DCHECK(callback || audio_callback_);
    audio_callback_ = callback;
    return 0;
  }

  int32_t StartPlayout() override {
    rtc::CritScope cs(&lock_);
    RTC_CHECK(renderer_);
    rendering_ = true;
    done_rendering_.Reset();
    return 0;
  }

  int32_t StopPlayout() override {
    rtc::CritScope cs(&lock_);
    rendering_ = false;
    done_rendering_.Set();
    return 0;
  }

  int32_t StartRecording() override {
    rtc::CritScope cs(&lock_);
    RTC_CHECK(capturer_);
    capturing_ = true;
    done_capturing_.Reset();
    return 0;
  }

  int32_t StopRecording() override {
    rtc::CritScope cs(&lock_);
    capturing_ = false;
    done_capturing_.Set();
    return 0;
  }

  bool Playing() const override {
    rtc::CritScope cs(&lock_);
    return rendering_;
  }

  bool Recording() const override {
    rtc::CritScope cs(&lock_);
    return capturing_;
  }

  // Blocks until the Renderer refuses to receive data.
  // Returns false if |timeout_ms| passes before that happens.
  bool WaitForPlayoutEnd(
      webrtc::TimeDelta timeout_ms = rtc::Event::kForever) override {
    return done_rendering_.Wait(timeout_ms);
  }

  // Blocks until the Recorder stops producing data.
  // Returns false if |timeout_ms| passes before that happens.
  bool WaitForRecordingEnd(
      webrtc::TimeDelta timeout_ms = rtc::Event::kForever) override {
    return done_capturing_.Wait(timeout_ms);
  }

private:
  void ProcessAudio() {
    int64_t time_us = rtc::TimeMicros();
    bool logged_once = false;
    for (;;) {
      {
        rtc::CritScope cs(&lock_);
        if (stop_thread_) {
          return;
        }
        // NOTE(mroberts): I've disabled this, as it was causing the following
        // error (and it's not really used by node-webrtc).
        //
        //   #
        //   # Fatal error in: ../../download/src/audio/audio_send_stream.cc,
        //   line 330 # last system error: 1 # Check failed:
        //   !race_checker.RaceDetected() # Aborted (core dumped)
        //
        /*
        if (capturing_) {
          // Capture 10ms of audio. 2 bytes per sample.
          const bool keep_capturing = capturer_->Capture(&recording_buffer_);
          uint32_t new_mic_level = 0;
          audio_callback_->RecordedDataIsAvailable(
              recording_buffer_.data(), recording_buffer_.size(), 2,
              capturer_->NumChannels(), capturer_->SamplingFrequency(), 0, 0,
              0, false, new_mic_level);
          if (!keep_capturing) {
            capturing_ = false;
            done_capturing_.Set();
          }
        }
        */
        if (rendering_) {
          size_t samples_out = 0;
          int64_t elapsed_time_ms = -1;
          int64_t ntp_time_ms = -1;

          // NOTE(jack): this code might also be racy, just like the above
          // commented-out block? Unfortunately:
          // * Commenting out this block causes the ondata callback to not fire
          //   (https://github.com/WonderInventions/node-webrtc/issues/2)
          // * Using the built-in webrtc::TestAudioDeviceModule causes audio to
          //   "not work"
          //   (https://github.com/WonderInventions/node-webrtc/issues/13)
          // So, I am going to uncomment out this block, and maybe in the
          // updates since then have made the race condition not happen?
          // Hoping beyond hope...
          const int sampling_frequency = renderer_->SamplingFrequency();
          if (audio_callback_) {
            audio_callback_->NeedMorePlayData(
                SamplesPerFrame(sampling_frequency), 2,
                renderer_->NumChannels(), sampling_frequency,
                playout_buffer_.data(), samples_out, &elapsed_time_ms,
                &ntp_time_ms);
          }

          const bool keep_rendering =
              renderer_->Render(rtc::ArrayView<const int16_t>(
                  playout_buffer_.data(), samples_out));
          if (!keep_rendering) {
            rendering_ = false;
            done_rendering_.Set();
          }
        }
      }
      // TODO(jack): change this to allow variable number of samples, not just
      // the hardcoded 10ms
      time_us += process_interval_us_;

      int64_t time_left_us = time_us - rtc::TimeMicros();
      if (time_left_us < 0) {
        if (!logged_once) {
          RTC_LOG(LS_ERROR) << "ProcessAudio is too slow";
          logged_once = true;
        }
      } else {
        while (time_left_us > 1000) {
          if (rtc::Thread::SleepMs(static_cast<int>(time_left_us / 1000))) {
            break;
          }
          time_left_us = time_us - rtc::TimeMicros();
        }
      }
    }
  }

  static void Run(void *obj) {
    static_cast<TestAudioDeviceModuleImpl *>(obj)->ProcessAudio();
  }

  const std::unique_ptr<webrtc::TestAudioDeviceModule::Capturer>
      capturer_ RTC_GUARDED_BY(lock_);
  const std::unique_ptr<webrtc::TestAudioDeviceModule::Renderer>
      renderer_ RTC_GUARDED_BY(lock_);
  const int64_t process_interval_us_;

  rtc::RecursiveCriticalSection lock_;
  webrtc::AudioTransport *audio_callback_ RTC_GUARDED_BY(lock_) = nullptr;
  bool rendering_ RTC_GUARDED_BY(lock_) = false;
  bool capturing_ RTC_GUARDED_BY(lock_) = false;
  rtc::Event done_rendering_;
  rtc::Event done_capturing_;

  std::vector<int16_t> playout_buffer_ RTC_GUARDED_BY(lock_);
  rtc::BufferT<int16_t> recording_buffer_ RTC_GUARDED_BY(lock_);

  std::unique_ptr<rtc::PlatformThread> thread_;
  bool stop_thread_ RTC_GUARDED_BY(lock_) = false;
};

class ZeroCapturerImpl final : public webrtc::TestAudioDeviceModule::Capturer {
public:
  ZeroCapturerImpl(int sampling_frequency_in_hz, int num_channels)
      : sampling_frequency_in_hz_(sampling_frequency_in_hz),
        num_channels_(num_channels) {}

  [[nodiscard]] int SamplingFrequency() const override {
    return sampling_frequency_in_hz_;
  }

  [[nodiscard]] int NumChannels() const override { return num_channels_; }

  bool Capture(rtc::BufferT<int16_t> *buffer) override {
    buffer->SetData(
        TestAudioDeviceModule::SamplesPerFrame(sampling_frequency_in_hz_) *
            num_channels_,
        [&](rtc::ArrayView<int16_t> data) {
          std::fill(data.begin(), data.end(), 0);
          return data.size();
        });
    return true;
  }

private:
  int sampling_frequency_in_hz_;
  const int num_channels_;
};

} // namespace

size_t TestAudioDeviceModule::SamplesPerFrame(int sampling_frequency_in_hz) {
  return rtc::CheckedDivExact(sampling_frequency_in_hz, kFramesPerSecond);
}

rtc::scoped_refptr<TestAudioDeviceModule>
TestAudioDeviceModule::CreateTestAudioDeviceModule(
    std::unique_ptr<webrtc::TestAudioDeviceModule::Capturer> capturer,
    std::unique_ptr<webrtc::TestAudioDeviceModule::Renderer> renderer,
    float speed) {
  return rtc::scoped_refptr<rtc::RefCountedObject<TestAudioDeviceModuleImpl>>(
      new rtc::RefCountedObject<TestAudioDeviceModuleImpl>(
          std::move(capturer), std::move(renderer), speed));
}

std::unique_ptr<webrtc::TestAudioDeviceModule::Capturer>
TestAudioDeviceModule::CreateZeroCapturer(

    int sampling_frequency_in_hz, int num_channels) {
  return std::make_unique<ZeroCapturerImpl>(sampling_frequency_in_hz,
                                            num_channels);
}

} // namespace node_webrtc
