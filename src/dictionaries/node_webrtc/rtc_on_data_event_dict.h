#pragma once

#include <cstdint>

// IWYU pragma: no_forward_declare node_webrtc::RTCOnDataEventDict

#define RTC_ON_DATA_EVENT_DICT RTCOnDataEventDict
#define RTC_ON_DATA_EVENT_DICT_LIST \
  DICT_REQUIRED(uint8_t*, samples, "samples") \
  DICT_DEFAULT(uint8_t, bitsPerSample, "bitsPerSample", 16) \
  DICT_REQUIRED(uint16_t, sampleRate, "sampleRate") \
  DICT_DEFAULT(uint8_t, channelCount, "channelCount", 1) \
  DICT_OPTIONAL(uint16_t, numberOfFrames, "numberOfFrames")

#define DICT(X) RTC_ON_DATA_EVENT_DICT ## X
#include "src/dictionaries/macros/def.h"
#include "src/dictionaries/macros/decls.h"
#undef DICT
