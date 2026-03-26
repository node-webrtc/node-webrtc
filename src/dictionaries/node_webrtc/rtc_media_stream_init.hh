#pragma once

#include <iosfwd>
#include <string>

// IWYU pragma: no_forward_declare node_webrtc::RTCMediaStreamInit
// IWYU pragma: no_include "src/dictionaries/macros/impls.hh"

#define RTC_MEDIA_STREAM_INIT RTCMediaStreamInit
#define RTC_MEDIA_STREAM_INIT_LIST DICT_REQUIRED(std::string, id, "id")

#define DICT(X) RTC_MEDIA_STREAM_INIT##X
#include "src/dictionaries/macros/def.hh"
// ordering
#include "src/dictionaries/macros/decls.hh"
#undef DICT
