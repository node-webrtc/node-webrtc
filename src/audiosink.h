#ifndef NODEWEBRTC_AUDIOSINK_H
#define NODEWEBRTC_AUDIOSINK_H
#include "webrtc/api/mediastreaminterface.h"
#include <functional>
#include <string>
using namespace std;

class AudioSink : public webrtc::AudioTrackSinkInterface
{
private:
  string label;
  function<void(const uint8_t* audio_data,
                int bits_per_sample, int sample_rate,
                size_t number_of_channels, size_t number_of_frames,
                string label)> onFrameCb;
public:
  AudioSink(function<void(const uint8_t* audio_data,
                          int bits_per_sample, int sample_rate,
                          size_t number_of_channels, size_t number_of_frames,
                          string label)> onFrame, string trackLabel);
  void OnData(const void* audio_data,
              int bits_per_sample,
              int sample_rate,
              size_t number_of_channels,
              size_t number_of_frames) override;
};

#endif