#pragma once

// IWYU pragma: no_forward_declare node_webrtc::RTCVideoSourceInit
// IWYU pragma: no_include "src/dictionaries/macros/impls.h"

#define RTC_VIDEO_SOURCE_INIT RTCVideoSourceInit
#define RTC_VIDEO_SOURCE_INIT_NAME "RTCVideoSourceInit"
#define RTC_VIDEO_SOURCE_INIT_LIST \
  DEFAULT(bool, isScreencast, "isScreencast", false) \
  OPTIONAL(bool, needsDenoising, "needsDenoising")

#define DICT(X) RTC_VIDEO_SOURCE_INIT ## X
#include "src/dictionaries/macros/def.h"
#include "src/dictionaries/macros/decls.h"
#undef DICT
