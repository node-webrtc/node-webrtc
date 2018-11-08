/*
 *  Copyright (c) 2004 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef SRC_WEBRTC_FAKEVIDEOCAPTURER_H_
#define SRC_WEBRTC_FAKEVIDEOCAPTURER_H_

#include <webrtc/api/video/i420_buffer.h>  // IWYU pragma: keep
#include <webrtc/api/video/video_rotation.h>
#include <webrtc/media/base/videocapturer.h>
#include <webrtc/rtc_base/task_queue_for_test.h>

namespace cricket {

struct VideoFormat;

}  // namespace cricket

namespace webrtc {

class VideoFrame;

}  // namespace webrtc

namespace node_webrtc {

class FakeFrameSource;  // IWYU pragma: keep

// Fake video capturer that allows the test to manually pump in frames.
class FakeVideoCapturer : public cricket::VideoCapturer {
 public:
  explicit FakeVideoCapturer(bool is_screencast);
  FakeVideoCapturer();

  ~FakeVideoCapturer() override;

  void ResetSupportedFormats(const std::vector<cricket::VideoFormat>& formats);
  virtual bool CaptureFrame();
  virtual bool CaptureCustomFrame(int width, int height);

  sigslot::signal1<FakeVideoCapturer*> SignalDestroyed;

  cricket::CaptureState Start(const cricket::VideoFormat& format) override;
  void Stop() override;
  bool IsRunning() override;
  bool IsScreencast() const override;
  bool GetPreferredFourccs(std::vector<uint32_t>* fourccs) override;

  void SetRotation(webrtc::VideoRotation rotation);

  webrtc::VideoRotation GetRotation();

 private:
  bool CaptureFrame(const webrtc::VideoFrame& frame);

  bool running_;
  const bool is_screencast_;
  // Duplicates FakeFrameSource::rotation_, but needed to support
  // SetRotation before Start.
  webrtc::VideoRotation rotation_;
  std::unique_ptr<node_webrtc::FakeFrameSource> frame_source_;
};

}  // namespace node_webrtc

#endif  // SRC_WEBRTC_FAKEVIDEOCAPTURER_H_
