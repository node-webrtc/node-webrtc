#include "src/dictionaries/node_webrtc/rtc_on_data_event_dict.hh"

#include <node-addon-api/napi.h>

#include "src/converters/object.hh"
#include "src/dictionaries/macros/napi.hh"
#include "src/functional/curry.hh"
#include "src/functional/operators.hh"
#include "src/functional/validation.hh"

namespace node_webrtc {

static Validation<RTC_ON_DATA_EVENT_DICT>
CreateRTCOnDataEventDict(Napi::ArrayBuffer samples, uint8_t bitsPerSample,
                         uint16_t sampleRate, uint8_t channelCount,
                         Maybe<uint16_t> maybeNumberOfFrames) {
  if (bitsPerSample != 16) {
    auto error =
        "Expected a .bitsPerSample of 16, not " + std::to_string(bitsPerSample);
    return Validation<RTC_ON_DATA_EVENT_DICT>::Invalid(error);
  }

  uint16_t numberOfFrames = sampleRate / 100; // 10 ms
  auto actualNumberOfFrames = maybeNumberOfFrames.FromMaybe(numberOfFrames);
  if (actualNumberOfFrames != numberOfFrames) {
    auto error = "Expected a .numberOfFrames of " +
                 std::to_string(numberOfFrames) + ", not " +
                 std::to_string(actualNumberOfFrames);
    return Validation<RTC_ON_DATA_EVENT_DICT>::Invalid(error);
  }

  auto actualByteLength = samples.ByteLength();
  auto expectedByteLength =
      static_cast<size_t>(channelCount * numberOfFrames * bitsPerSample / 8);
  if (actualByteLength != expectedByteLength) {
    auto error = "Expected a .byteLength of " +
                 std::to_string(expectedByteLength) + ", not " +
                 std::to_string(actualByteLength);
    return Validation<RTC_ON_DATA_EVENT_DICT>::Invalid(error);
  }

  std::unique_ptr<uint8_t[]> samplesCopy(new uint8_t[actualByteLength]);
  if (!samplesCopy) {
    auto error = "Failed to copy samples";
    return Validation<RTC_ON_DATA_EVENT_DICT>::Invalid(error);
  }
  memcpy(samplesCopy.get(), samples.Data(), actualByteLength);

  RTC_ON_DATA_EVENT_DICT dict = {samplesCopy.release(), bitsPerSample,
                                 sampleRate, channelCount,
                                 MakeJust<uint16_t>(numberOfFrames)};

  return Pure(dict);
}

FROM_NAPI_IMPL(RTC_ON_DATA_EVENT_DICT, value) {
  return From<Napi::Object>(value).FlatMap<RTC_ON_DATA_EVENT_DICT>(
      [](auto object) {
        return Validation<RTC_ON_DATA_EVENT_DICT>::Join(
            curry(CreateRTCOnDataEventDict) %
            GetRequired<Napi::ArrayBuffer>(object, "samples") *
            GetOptional<uint8_t>(object, "bitsPerSample", 16) *
            GetRequired<uint16_t>(object, "sampleRate") *
            GetOptional<uint8_t>(object, "channelCount", 1) *
            GetOptional<uint16_t>(object, "numberOfFrames"));
      });
}

TO_NAPI_IMPL(RTC_ON_DATA_EVENT_DICT, pair) {
  auto env = pair.first;
  Napi::EscapableHandleScope scope(env);

  auto dict = pair.second;
  std::unique_ptr<uint8_t> samples(dict.samples);

  if (dict.numberOfFrames.IsNothing()) {
    return Validation<Napi::Value>::Invalid("numberOfFrames not provided");
  }
  auto numberOfFrames = dict.numberOfFrames.UnsafeFromJust();

  auto length = dict.channelCount * numberOfFrames;
  auto byteLength = length * dict.bitsPerSample / 8;
  auto maybeArrayBuffer = Napi::ArrayBuffer::New(
      env, samples.release(), byteLength,
      [](Napi::Env, void *samples) { delete static_cast<uint8_t *>(samples); });
  if (maybeArrayBuffer.Env().IsExceptionPending()) {
    return Validation<Napi::Value>::Invalid(
        maybeArrayBuffer.Env().GetAndClearPendingException().Message());
  }

  // Slicing concerns can be ignored since Napi::Value stores enough type
  // information to reconstruct them.
  Napi::Value maybeSamples;
  switch (dict.bitsPerSample) {
  case 8:
    maybeSamples = Napi::Int8Array::New(env, length, maybeArrayBuffer,
                                        0); // NOLINT(cppcoreguidelines-slicing)
    break;
  case 16:
    maybeSamples = Napi::Int16Array::New(
        env, length, maybeArrayBuffer, 0); // NOLINT(cppcoreguidelines-slicing)
    break;
  case 32:
    maybeSamples = Napi::Int32Array::New(
        env, length, maybeArrayBuffer, 0); // NOLINT(cppcoreguidelines-slicing)
    break;
  default:
    maybeSamples = Napi::Uint8Array::New(
        env, length, maybeArrayBuffer, 0); // NOLINT(cppcoreguidelines-slicing)
    break;
  }
  if (maybeSamples.Env().IsExceptionPending()) {
    return Validation<Napi::Value>::Invalid(
        maybeSamples.Env().GetAndClearPendingException().Message());
  }

  NODE_WEBRTC_CREATE_OBJECT_OR_RETURN(env, object)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "samples", maybeSamples)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "bitsPerSample",
                                        dict.bitsPerSample)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "sampleRate",
                                        dict.sampleRate)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "channelCount",
                                        dict.channelCount)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "numberOfFrames",
                                        dict.numberOfFrames)
  return Pure(scope.Escape(object));
}

} // namespace node_webrtc
