#pragma once

#include <webrtc/api/media_stream_interface.h>

// IWYU pragma: no_include "src/enums/macros/impls.h"

#define TRACK_STATE webrtc::MediaStreamTrackInterface::TrackState
#define TRACK_STATE_NAME "MediaStreamTrackState"
#define TRACK_STATE_LIST \
  SUPPORTED(TRACK_STATE, kEnded, "ended") \
  SUPPORTED(TRACK_STATE, kLive, "live")

#define ENUM(X) TRACK_STATE ## X
#include "src/enums/macros/decls.h"
#undef ENUM
