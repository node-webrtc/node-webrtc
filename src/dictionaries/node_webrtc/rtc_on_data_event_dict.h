#pragma once

#include <cstdint>  // IWYU pragma: keep

// IWYU pragma: no_forward_declare node_webrtc::RTCOnDataEventDict
// IWYU pragma: no_include <_types/_uint16_t.h>
// IWYU pragma: no_include <_types/_uint8_t.h>

#define RTC_ON_DATA_EVENT_DICT RTCOnDataEventDict
#define RTC_ON_DATA_EVENT_DICT_NAME "RTCOnDataEventDict"
#define RTC_ON_DATA_EVENT_DICT_LIST \
  REQUIRED(uint8_t*, samples, "samples") \
  DEFAULT(uint8_t, bitsPerSample, "bitsPerSample", 16) \
  REQUIRED(uint16_t, sampleRate, "sampleRate") \
  DEFAULT(uint8_t, channelCount, "channelCount", 1) \
  OPTIONAL(uint16_t, numberOfFrames, "numberOfFrames")

#define DICT(X) RTC_ON_DATA_EVENT_DICT ## X
#include "src/dictionaries/macros/def.h"
#include "src/dictionaries/macros/decls.h"
#undef DICT
