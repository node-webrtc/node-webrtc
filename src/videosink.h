#ifndef NODEWEBRTC_VIDEOSINK_H
#define NODEWEBRTC_VIDEOSINK_H
#include "webrtc/api/mediastreaminterface.h"
#include <functional>
#include <string>
using namespace std;

class VideoSink : public rtc::VideoSinkInterface<webrtc::VideoFrame>
{
private:
  string label;
  function<void(const webrtc::VideoFrame&, string label)> onFrameCb;
public:
  VideoSink(function<void(const webrtc::VideoFrame&, string label)> onFrame, string trackLabel);
  void OnFrame(const webrtc::VideoFrame& frame) override;
};

#endif