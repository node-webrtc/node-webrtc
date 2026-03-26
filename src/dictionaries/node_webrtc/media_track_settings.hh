#pragma once

#include <cstdbool>
#include <cstdint>
#include <iosfwd>
#include <string>

// See https://developer.mozilla.org/en-US/docs/Web/API/MediaTrackSettings
// FIXME(jack): clean these up & add more validation. Currently v hacky

#define AUDIO_MEDIA_TRACK_SETTINGS AudioMediaTrackSettings
#define AUDIO_MEDIA_TRACK_SETTINGS_LIST                                        \
  DICT_REQUIRED(std::string, deviceId, "deviceId")                             \
  DICT_REQUIRED(std::string, groupId, "groupId")                               \
  DICT_REQUIRED(bool, autoGainControl, "autoGainControl")                      \
  DICT_REQUIRED(bool, echoCancellation, "echoCancellation")                    \
  DICT_REQUIRED(bool, noiseSuppression, "noiseSuppression")                    \
  DICT_REQUIRED(uint64_t, channelCount, "channelCount")                        \
  DICT_REQUIRED(uint64_t, sampleRate, "sampleRate")                            \
  DICT_REQUIRED(uint64_t, sampleSize, "sampleSize")                            \
  DICT_REQUIRED(double, latency, "latency")                                    \
  DICT_REQUIRED(double, volume, "volume")

#define DICT(X) AUDIO_MEDIA_TRACK_SETTINGS##X
#include "src/dictionaries/macros/def.hh"
// ordering
#include "src/dictionaries/macros/decls.hh"
#undef DICT

#define VIDEO_MEDIA_TRACK_SETTINGS VideoMediaTrackSettings
#define VIDEO_MEDIA_TRACK_SETTINGS_LIST                                        \
  DICT_REQUIRED(std::string, deviceId, "deviceId")                             \
  DICT_REQUIRED(std::string, groupId, "groupId")                               \
  DICT_REQUIRED(uint64_t, height, "height")                                    \
  DICT_REQUIRED(uint64_t, width, "width")                                      \
  DICT_REQUIRED(double, aspectRatio, "aspectRatio")                            \
  DICT_REQUIRED(double, frameRate, "frameRate")                                \
  DICT_REQUIRED(std::string, facingMode, "facingMode")
// FIXME(jack): especially here, where this should really be an enum instead

#define DICT(X) VIDEO_MEDIA_TRACK_SETTINGS##X
#include "src/dictionaries/macros/def.hh"
// ordering
#include "src/dictionaries/macros/decls.hh"
#undef DICT
