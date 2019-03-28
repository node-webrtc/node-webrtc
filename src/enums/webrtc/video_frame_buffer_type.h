#pragma once

#include <webrtc/api/video/video_frame_buffer.h>

// IWYU pragma: no_include "src/enums/macros/impls.h"

#define VIDEO_FRAME_BUFFER_TYPE webrtc::VideoFrameBuffer::Type
#define VIDEO_FRAME_BUFFER_TYPE_NAME "VideoFrameBufferType"
#define VIDEO_FRAME_BUFFER_TYPE_LIST \
  UNSUPPORTED(VIDEO_FRAME_BUFFER_TYPE, kNative, "native", "\"native\" is not a valid VideoFrameBufferType") \
  SUPPORTED(VIDEO_FRAME_BUFFER_TYPE, kI420, "I420") \
  SUPPORTED(VIDEO_FRAME_BUFFER_TYPE, kI420A, "I420A") \
  SUPPORTED(VIDEO_FRAME_BUFFER_TYPE, kI444, "I444") \
  SUPPORTED(VIDEO_FRAME_BUFFER_TYPE, kI010, "I010")

#define ENUM(X) VIDEO_FRAME_BUFFER_TYPE ## X
#include "src/enums/macros/decls.h"  // IWYU pragma: keep
#undef ENUM
