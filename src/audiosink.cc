#include "audiosink.h"

#include <iostream>
using namespace std;

AudioSink::AudioSink(function<void(const uint8_t* audio_data,
                                   int bits_per_sample, int sample_rate,
                                   size_t number_of_channels, size_t number_of_frames,
                                   string label)> onFrame, string label) : label(label)
{
  onFrameCb = onFrame;
}

void AudioSink::OnData(const void* audio_data,
                       int bits_per_sample, int sample_rate,
                       size_t number_of_channels, size_t number_of_frames)
{
  onFrameCb(static_cast<const uint8_t*>(audio_data), bits_per_sample, sample_rate, number_of_channels, number_of_frames, label);
}
