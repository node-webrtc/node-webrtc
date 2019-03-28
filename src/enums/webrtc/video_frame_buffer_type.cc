#include "src/enums/webrtc/video_frame_buffer_type.h"

#define ENUM(X) VIDEO_FRAME_BUFFER_TYPE ## X
#include "src/enums/macros/impls.h"
#undef ENUM
