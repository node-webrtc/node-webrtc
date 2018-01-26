#include "videosink.h"

VideoSink::VideoSink(function<void(const webrtc::VideoFrame&, string label)> onFrame, string label)
{
  onFrameCb = onFrame;
}

void VideoSink::OnFrame(const webrtc::VideoFrame& frame)
{
  onFrameCb(frame, label);
}