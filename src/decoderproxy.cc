#include "decoderproxy.h"

#include <iostream>
#include <sstream>
#include <stdio.h>
using namespace std;

#include "webrtc/base/file.h"
#define LIMIT_BYTE_SIZE 1024*1024

std::map<std::string, std::function<void(const webrtc::EncodedImage&, std::string label)> > encodedImageCallbacks;

void DecoderProxy::RegisterProxyCallback(std::string label,
                                                   std::function<void(const webrtc::EncodedImage&,
                                                                      std::string label)> encodedImageCallback) {
  encodedImageCallbacks[label] = encodedImageCallback;
}
DecoderProxy* DecoderProxy::Create(std::string label, VideoDecoder* decoder) {

  return new DecoderProxy(label, decoder);
}

DecoderProxy::DecoderProxy(std::string label, VideoDecoder* decoder) : label(label), decoder(decoder) {
}
DecoderProxy::~DecoderProxy() {
  delete decoder;
}
int32_t DecoderProxy::InitDecode(const webrtc::VideoCodec* codec_settings,
                           int32_t number_of_cores) {
  return decoder->InitDecode(codec_settings, number_of_cores);
}

int32_t DecoderProxy::Decode(const webrtc::EncodedImage& input_image,
                       bool missing_frames,
                       const webrtc::RTPFragmentationHeader* fragmentation,
                       const webrtc::CodecSpecificInfo* codec_specific_info,
                       int64_t render_time_ms) {
  if(encodedImageCallbacks.find(label) != encodedImageCallbacks.end()) {
    encodedImageCallbacks[label](input_image, label);
  }
  return decoder->Decode(input_image, missing_frames, fragmentation, codec_specific_info, render_time_ms);
}

int32_t DecoderProxy::RegisterDecodeCompleteCallback(
    webrtc::DecodedImageCallback* callback) {
  return decoder->RegisterDecodeCompleteCallback(callback);
}

int32_t DecoderProxy::Release() {
  return decoder->Release();
}
