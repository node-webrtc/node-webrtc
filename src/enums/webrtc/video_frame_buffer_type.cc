#include "src/enums/webrtc/video_frame_buffer_type.hh"

#define ENUM(X) VIDEO_FRAME_BUFFER_TYPE##X
#include "src/enums/macros/impls.hh"
#undef ENUM
