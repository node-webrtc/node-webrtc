#ifndef NODEVP8_DECODER_PROXY_H
#define NODEVP8_DECODER_PROXY_H

#include "webrtc/modules/video_coding/codecs/vp8/include/vp8.h"  // nogncheck
#include "webrtc/modules/video_coding/utility/ivf_file_writer.h"
#include <memory>
#include <string>
#include <functional>
#include <map>

class DecoderProxy : public webrtc::VideoDecoder {
public:
  static void RegisterProxyCallback(std::string label, std::function<void(const webrtc::EncodedImage&, std::string label)> encodedImageCallback);
  static DecoderProxy* Create(std::string label, VideoDecoder* decoder);
  virtual int32_t InitDecode(const webrtc::VideoCodec* codec_settings,
                             int32_t number_of_cores);

  virtual int32_t Decode(const webrtc::EncodedImage& input_image,
                         bool missing_frames,
                         const webrtc::RTPFragmentationHeader* fragmentation,
                         const webrtc::CodecSpecificInfo* codec_specific_info = NULL,
                         int64_t render_time_ms = -1);

  virtual int32_t RegisterDecodeCompleteCallback(
      webrtc::DecodedImageCallback* callback);

  virtual int32_t Release();

  // Returns true if the decoder prefer to decode frames late.
  // That is, it can not decode infinite number of frames before the decoded
  // frame is consumed.
  virtual bool PrefersLateDecoding() const { return true; }

  virtual const char* ImplementationName() const { return "proxy"; }
private:
  DecoderProxy(std::string label, webrtc::VideoDecoder* decoder);
  ~DecoderProxy();
  webrtc::VideoDecoder* decoder = nullptr;

  std::string label = "UNKNOWN";
};

#endif