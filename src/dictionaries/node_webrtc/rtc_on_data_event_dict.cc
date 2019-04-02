#include "src/dictionaries/node_webrtc/rtc_on_data_event_dict.h"

#include <node-addon-api/napi.h>

#include "src/converters/object.h"
#include "src/dictionaries/macros/napi.h"
#include "src/functional/curry.h"
#include "src/functional/operators.h"
#include "src/functional/validation.h"

namespace node_webrtc {

static Validation<RTC_ON_DATA_EVENT_DICT> CreateRTCOnDataEventDict(
    v8::ArrayBuffer::Contents samples,
    uint8_t bitsPerSample,
    uint16_t sampleRate,
    uint8_t channelCount,
    Maybe<uint16_t> maybeNumberOfFrames) {
  if (bitsPerSample != 16) {
    auto error = "Expected a .bitsPerSample of 16, not " + std::to_string(bitsPerSample);
    return Validation<RTC_ON_DATA_EVENT_DICT>::Invalid(error);
  }

  uint16_t numberOfFrames = sampleRate / 100;  // 10 ms
  auto actualNumberOfFrames = maybeNumberOfFrames.FromMaybe(numberOfFrames);
  if (actualNumberOfFrames != numberOfFrames) {
    auto error = "Expected a .numberOfFrames of " + std::to_string(numberOfFrames) + ", not " + std::to_string(actualNumberOfFrames);
    return Validation<RTC_ON_DATA_EVENT_DICT>::Invalid(error);
  }

  auto actualByteLength = samples.ByteLength();
  // NOLINTNEXTLINE
  auto expectedByteLength = static_cast<size_t>(channelCount * numberOfFrames * bitsPerSample / 8);
  if (actualByteLength != expectedByteLength) {
    auto error = "Expected a .byteLength of " + std::to_string(expectedByteLength) + ", not " +
        std::to_string(actualByteLength);
    return Validation<RTC_ON_DATA_EVENT_DICT>::Invalid(error);
  }

  std::unique_ptr<uint8_t[]> samplesCopy(new uint8_t[actualByteLength]);
  if (!samplesCopy) {
    auto error = "Failed to copy samples";
    return Validation<RTC_ON_DATA_EVENT_DICT>::Invalid(error);
  }
  memcpy(samplesCopy.get(), samples.Data(), actualByteLength);

  RTC_ON_DATA_EVENT_DICT dict = {
    samplesCopy.release(),
    bitsPerSample,
    sampleRate,
    channelCount,
    MakeJust<uint16_t>(numberOfFrames)
  };

  return Pure(dict);
}

namespace napi {

static Validation<RTC_ON_DATA_EVENT_DICT> CreateRTCOnDataEventDict(
    Napi::ArrayBuffer samples,
    uint8_t bitsPerSample,
    uint16_t sampleRate,
    uint8_t channelCount,
    Maybe<uint16_t> maybeNumberOfFrames) {
  if (bitsPerSample != 16) {
    auto error = "Expected a .bitsPerSample of 16, not " + std::to_string(bitsPerSample);
    return Validation<RTC_ON_DATA_EVENT_DICT>::Invalid(error);
  }

  uint16_t numberOfFrames = sampleRate / 100;  // 10 ms
  auto actualNumberOfFrames = maybeNumberOfFrames.FromMaybe(numberOfFrames);
  if (actualNumberOfFrames != numberOfFrames) {
    auto error = "Expected a .numberOfFrames of " + std::to_string(numberOfFrames) + ", not " + std::to_string(actualNumberOfFrames);
    return Validation<RTC_ON_DATA_EVENT_DICT>::Invalid(error);
  }

  auto actualByteLength = samples.ByteLength();
  // NOLINTNEXTLINE
  auto expectedByteLength = static_cast<size_t>(channelCount * numberOfFrames * bitsPerSample / 8);
  if (actualByteLength != expectedByteLength) {
    auto error = "Expected a .byteLength of " + std::to_string(expectedByteLength) + ", not " +
        std::to_string(actualByteLength);
    return Validation<RTC_ON_DATA_EVENT_DICT>::Invalid(error);
  }

  std::unique_ptr<uint8_t[]> samplesCopy(new uint8_t[actualByteLength]);
  if (!samplesCopy) {
    auto error = "Failed to copy samples";
    return Validation<RTC_ON_DATA_EVENT_DICT>::Invalid(error);
  }
  memcpy(samplesCopy.get(), samples.Data(), actualByteLength);

  RTC_ON_DATA_EVENT_DICT dict = {
    samplesCopy.release(),
    bitsPerSample,
    sampleRate,
    channelCount,
    MakeJust<uint16_t>(numberOfFrames)
  };

  return Pure(dict);
}

}  // namespace napi

FROM_JS_IMPL(RTC_ON_DATA_EVENT_DICT, value) {
  return From<v8::Local<v8::Object>>(value).FlatMap<RTC_ON_DATA_EVENT_DICT>([](auto object) {
    return Validation<RTC_ON_DATA_EVENT_DICT>::Join(curry(CreateRTCOnDataEventDict)
            % GetRequired<v8::ArrayBuffer::Contents>(object, "samples")
            * GetOptional<uint8_t>(object, "bitsPerSample", 16)
            * GetRequired<uint16_t>(object, "sampleRate")
            * GetOptional<uint8_t>(object, "channelCount", 1)
            * GetOptional<uint16_t>(object, "numberOfFrames"));
  });
}

FROM_NAPI_IMPL(RTC_ON_DATA_EVENT_DICT, value) {
  return From<Napi::Object>(value).FlatMap<RTC_ON_DATA_EVENT_DICT>([](auto object) {
    return Validation<RTC_ON_DATA_EVENT_DICT>::Join(curry(napi::CreateRTCOnDataEventDict)
            % napi::GetRequired<Napi::ArrayBuffer>(object, "samples")
            * napi::GetOptional<uint8_t>(object, "bitsPerSample", 16)
            * napi::GetRequired<uint16_t>(object, "sampleRate")
            * napi::GetOptional<uint8_t>(object, "channelCount", 1)
            * napi::GetOptional<uint16_t>(object, "numberOfFrames"));
  });
}

TO_JS_IMPL(RTC_ON_DATA_EVENT_DICT, dict) {
  Nan::EscapableHandleScope scope;

  if (dict.numberOfFrames.IsNothing()) {
    return Validation<v8::Local<v8::Value>>::Invalid("numberOfFrames not provided");
  }
  auto numberOfFrames = dict.numberOfFrames.UnsafeFromJust();

  auto isolate = Nan::GetCurrentContext()->GetIsolate();
  auto length = dict.channelCount * numberOfFrames;
  auto byteLength = length * dict.bitsPerSample / 8;
  auto arrayBuffer = v8::ArrayBuffer::New(isolate, dict.samples, byteLength, v8::ArrayBufferCreationMode::kInternalized);

  v8::Local<v8::Value> samples;
  switch (dict.bitsPerSample) {
    case 8:
      samples = v8::Int8Array::New(arrayBuffer, 0, length);
      break;
    case 16:
      samples = v8::Int16Array::New(arrayBuffer, 0, length);
      break;
    case 32:
      samples = v8::Int32Array::New(arrayBuffer, 0, length);
      break;
    default:
      samples = v8::Uint8Array::New(arrayBuffer, 0, byteLength);
  }

  auto object = Nan::New<v8::Object>();
  object->Set(Nan::New("samples").ToLocalChecked(), samples);
  object->Set(Nan::New("bitsPerSample").ToLocalChecked(), From<v8::Local<v8::Value>>(dict.bitsPerSample).UnsafeFromValid());
  object->Set(Nan::New("sampleRate").ToLocalChecked(), From<v8::Local<v8::Value>>(dict.sampleRate).UnsafeFromValid());
  object->Set(Nan::New("channelCount").ToLocalChecked(), From<v8::Local<v8::Value>>(dict.channelCount).UnsafeFromValid());
  object->Set(Nan::New("numberOfFrames").ToLocalChecked(), From<v8::Local<v8::Value>>(numberOfFrames).UnsafeFromValid());
  return Pure(scope.Escape(object).As<v8::Value>());
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
  auto maybeArrayBuffer = Napi::ArrayBuffer::New(env, byteLength);
  if (maybeArrayBuffer.Env().IsExceptionPending()) {
    return Validation<Napi::Value>::Invalid(maybeArrayBuffer.Env().GetAndClearPendingException().Message());
  }

  memcpy(maybeArrayBuffer.Data(), samples.get(), byteLength);

  Napi::Value maybeSamples;
  switch (dict.bitsPerSample) {
    case 8:
      maybeSamples = Napi::Int8Array::New(env, length, maybeArrayBuffer, 0);  // NOLINT
      break;
    case 16:
      maybeSamples = Napi::Int16Array::New(env, length, maybeArrayBuffer, 0);  // NOLINT
      break;
    case 32:
      maybeSamples = Napi::Int32Array::New(env, length, maybeArrayBuffer, 0);  // NOLINT
      break;
    default:
      maybeSamples = Napi::Uint8Array::New(env, length, maybeArrayBuffer, 0);  // NOLINT
  }
  if (maybeSamples.Env().IsExceptionPending()) {
    return Validation<Napi::Value>::Invalid(maybeSamples.Env().GetAndClearPendingException().Message());
  }

  NODE_WEBRTC_CREATE_OBJECT_OR_RETURN(env, object)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "samples", maybeSamples)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "bitsPerSample", dict.bitsPerSample)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "sampleRate", dict.sampleRate)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "channelCount", dict.channelCount)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "numberOfFrames", dict.numberOfFrames)
  return Pure(scope.Escape(object));
}

}  // namespace node_webrtc
