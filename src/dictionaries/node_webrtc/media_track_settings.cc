#include "src/dictionaries/node_webrtc/media_track_settings.hh"

#include <node-addon-api/napi.h>

#include "src/converters/object.hh"
#include "src/dictionaries/macros/napi.hh"
#include "src/functional/validation.hh"

namespace node_webrtc {

#define AUDIO_MEDIA_TRACK_SETTINGS_FN CreateAudioMediaTrackSettings

namespace {
Validation<AUDIO_MEDIA_TRACK_SETTINGS> AUDIO_MEDIA_TRACK_SETTINGS_FN(
    const std::string &deviceId, const std::string &groupId,
    const bool autoGainControl, const bool echoCancellation,
    const bool noiseSuppression, const uint64_t channelCount,
    const uint64_t sampleRate, const uint64_t sampleSize, const double latency,
    const double volume) {
  return Pure<AUDIO_MEDIA_TRACK_SETTINGS>({
      deviceId,
      groupId,
      autoGainControl,
      echoCancellation,
      noiseSuppression,
      channelCount,
      sampleRate,
      sampleSize,
      latency,
      volume,
  });
}
} // namespace

TO_NAPI_IMPL(AUDIO_MEDIA_TRACK_SETTINGS, pair) {
  auto env = pair.first;
  Napi::EscapableHandleScope scope(env);

  auto dict = pair.second;

  NODE_WEBRTC_CREATE_OBJECT_OR_RETURN(env, object)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "deviceId", dict.deviceId)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "groupId", dict.groupId)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "autoGainControl",
                                        dict.autoGainControl)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "echoCancellation",
                                        dict.echoCancellation)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "noiseSuppression",
                                        dict.noiseSuppression)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "channelCount",
                                        dict.channelCount)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "sampleRate",
                                        dict.sampleRate)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "sampleSize",
                                        dict.sampleSize)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "latency", dict.latency)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "volume", dict.volume)
  return Pure(scope.Escape(object));
}

#define VIDEO_MEDIA_TRACK_SETTINGS_FN CreateVideoMediaTrackSettings

namespace {
Validation<VIDEO_MEDIA_TRACK_SETTINGS> VIDEO_MEDIA_TRACK_SETTINGS_FN(
    const std::string &deviceId, const std::string &groupId,
    const uint64_t height, const uint64_t width, const double aspectRatio,
    const double frameRate, const std::string &facingMode) {
  return Pure<VIDEO_MEDIA_TRACK_SETTINGS>({
      deviceId,
      groupId,
      height,
      width,
      aspectRatio,
      frameRate,
      facingMode,
  });
}
} // namespace

TO_NAPI_IMPL(VIDEO_MEDIA_TRACK_SETTINGS, pair) {
  auto env = pair.first;
  Napi::EscapableHandleScope scope(env);

  auto dict = pair.second;

  NODE_WEBRTC_CREATE_OBJECT_OR_RETURN(env, object)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "deviceId", dict.deviceId)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "groupId", dict.groupId)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "height", dict.height)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "width", dict.width)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "aspectRatio",
                                        dict.aspectRatio)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "frameRate",
                                        dict.frameRate)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "facingMode",
                                        dict.facingMode)
  return Pure(scope.Escape(object));
}
} // namespace node_webrtc

#define DICT(X) AUDIO_MEDIA_TRACK_SETTINGS##X
#include "src/dictionaries/macros/impls.hh"
#undef DICT

#define DICT(X) VIDEO_MEDIA_TRACK_SETTINGS##X
#include "src/dictionaries/macros/impls.hh"
#undef DICT
