#pragma once

#include "src/converters.h"

namespace rtc { template <typename T> class scoped_refptr; }

namespace webrtc {

class I420Buffer;
class I420BufferInterface;
class VideoFrameBuffer;

}  // namespace webrtc

namespace node_webrtc {

class I420ImageData;

DECLARE_CONVERTER(I420ImageData, rtc::scoped_refptr<webrtc::I420Buffer>)
DECLARE_FROM_JS(rtc::scoped_refptr<webrtc::I420Buffer>)
DECLARE_TO_JS(rtc::scoped_refptr<webrtc::I420BufferInterface>)
DECLARE_TO_JS(rtc::scoped_refptr<webrtc::VideoFrameBuffer>)

}  // namespace node_webrtc
