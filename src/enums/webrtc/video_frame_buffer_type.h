#pragma once

#include <webrtc/api/video/video_frame_buffer.h>

// IWYU pragma: no_include "src/enums/macros/impls.h"

#define VIDEO_FRAME_BUFFER_TYPE webrtc::VideoFrameBuffer::Type
#define VIDEO_FRAME_BUFFER_TYPE_NAME "VideoFrameBufferType"
#define VIDEO_FRAME_BUFFER_TYPE_LIST \
  ENUM_UNSUPPORTED(VIDEO_FRAME_BUFFER_TYPE::kNative, "native", "\"native\" is not a valid VideoFrameBufferType") \
  ENUM_SUPPORTED(VIDEO_FRAME_BUFFER_TYPE::kI420, "I420") \
  ENUM_SUPPORTED(VIDEO_FRAME_BUFFER_TYPE::kI420A, "I420A") \
  ENUM_SUPPORTED(VIDEO_FRAME_BUFFER_TYPE::kI444, "I444") \
  ENUM_SUPPORTED(VIDEO_FRAME_BUFFER_TYPE::kI010, "I010") \
  ENUM_SUPPORTED(VIDEO_FRAME_BUFFER_TYPE::kNV12, "NV12")

#define ENUM(X) VIDEO_FRAME_BUFFER_TYPE ## X
#include "src/enums/macros/decls.h"
#undef ENUM
