#pragma once

#include "src/converters.h"
#include "src/converters/v8.h"

namespace rtc { template <typename T> class scoped_refptr; }
namespace webrtc { class I420Buffer; }
namespace webrtc { class I420BufferInterface; }
namespace webrtc { class VideoFrameBuffer; }

namespace node_webrtc {

class I420ImageData;

DECLARE_CONVERTER(I420ImageData, rtc::scoped_refptr<webrtc::I420Buffer>)
DECLARE_FROM_JS(rtc::scoped_refptr<webrtc::I420Buffer>)
DECLARE_TO_JS(rtc::scoped_refptr<webrtc::I420BufferInterface>)
DECLARE_TO_JS(rtc::scoped_refptr<webrtc::VideoFrameBuffer>)

}  // namespace node_webrtc
