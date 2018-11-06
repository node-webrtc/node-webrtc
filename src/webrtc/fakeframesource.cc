/*
 *  Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include "src/webrtc/fakeframesource.h"

#include <webrtc/api/video/i420_buffer.h>  // IWYU pragma: keep
#include <webrtc/rtc_base/checks.h>
#include <webrtc/rtc_base/scoped_ref_ptr.h>

// IWYU pragma: no_include <api/video/video_frame.h>
// IWYU pragma: no_include <api/video/video_rotation.h>

using node_webrtc::FakeFrameSource;

FakeFrameSource::FakeFrameSource(int width, int height, int interval_us)
  : width_(width), height_(height), interval_us_(interval_us) {
  RTC_CHECK_GT(width_, 0);
  RTC_CHECK_GT(height_, 0);
  RTC_CHECK_GT(interval_us_, 0);
}

webrtc::VideoRotation FakeFrameSource::GetRotation() const {
  return rotation_;
}

void FakeFrameSource::SetRotation(webrtc::VideoRotation rotation) {
  rotation_ = rotation;
}

webrtc::VideoFrame FakeFrameSource::GetFrameRotationApplied() {
  switch (rotation_) {
    case webrtc::kVideoRotation_0:
    case webrtc::kVideoRotation_180:
      return GetFrame(width_, height_, webrtc::kVideoRotation_0, interval_us_);
    case webrtc::kVideoRotation_90:
    case webrtc::kVideoRotation_270:
      return GetFrame(height_, width_, webrtc::kVideoRotation_0, interval_us_);
  }
  RTC_NOTREACHED() << "Invalid rotation value: " << static_cast<int>(rotation_);
  // Without this return, the Windows Visual Studio compiler complains
  // "not all control paths return a value".
  return GetFrame();
}

webrtc::VideoFrame FakeFrameSource::GetFrame() {
  return GetFrame(width_, height_, rotation_, interval_us_);
}

webrtc::VideoFrame FakeFrameSource::GetFrame(int width,
    int height,
    webrtc::VideoRotation rotation,
    int interval_us) {
  RTC_CHECK_GT(width, 0);
  RTC_CHECK_GT(height, 0);
  RTC_CHECK_GT(interval_us, 0);

  rtc::scoped_refptr<webrtc::I420Buffer> buffer(
      webrtc::I420Buffer::Create(width, height));

  buffer->InitializeData();
  webrtc::VideoFrame frame =
      webrtc::VideoFrame(buffer, rotation, next_timestamp_us_);

  next_timestamp_us_ += interval_us;
  return frame;
}
