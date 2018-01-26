#ifndef NODEWEBRTC_DECODERFACTORY_H
#define NODEWEBRTC_DECODERFACTORY_H
#include "webrtc/media/engine/webrtcvideodecoderfactory.h"

#include <iostream>

class NodeDecoderFactory : public cricket::WebRtcVideoDecoderFactory {
  virtual webrtc::VideoDecoder* CreateVideoDecoderWithParams(
      webrtc::VideoCodecType type,
      cricket::VideoDecoderParams params) {
    std::cout << "CREATE VIDEO DECODER WITH PARAMS " << params.receive_stream_id << std::endl;
    return CreateVideoDecoder(type, params);
  }
  virtual webrtc::VideoDecoder* CreateVideoDecoder(
      webrtc::VideoCodecType type, cricket::VideoDecoderParams params);

  virtual webrtc::VideoDecoder* CreateVideoDecoder(
      webrtc::VideoCodecType type);
  virtual void DestroyVideoDecoder(webrtc::VideoDecoder* decoder);
};
#endif