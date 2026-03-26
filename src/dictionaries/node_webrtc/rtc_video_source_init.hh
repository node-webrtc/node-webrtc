#pragma once

// IWYU pragma: no_forward_declare node_webrtc::RTCVideoSourceInit
// IWYU pragma: no_include "src/dictionaries/macros/impls.hh"

#define RTC_VIDEO_SOURCE_INIT RTCVideoSourceInit
#define RTC_VIDEO_SOURCE_INIT_LIST                                             \
  DICT_DEFAULT(bool, isScreencast, "isScreencast", false)                      \
  DICT_OPTIONAL(bool, needsDenoising, "needsDenoising")

#define DICT(X) RTC_VIDEO_SOURCE_INIT##X
#include "src/dictionaries/macros/def.hh"
// ordering
#include "src/dictionaries/macros/decls.hh"
#undef DICT
