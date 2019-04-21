#include "src/dictionaries/webrtc/rtc_stats_report.h"

#include <node-addon-api/napi.h>
#include <webrtc/api/scoped_refptr.h>  // IWYU pragma: keep
#include <webrtc/api/stats/rtc_stats_report.h>  // IWYU pragma: keep

#include "src/converters/object.h"
#include "src/dictionaries/webrtc/rtc_stats.h"  // IWYU pragma: keep
#include "src/functional/validation.h"

namespace node_webrtc {

static Validation<Napi::Object> CreateMap(Napi::Env env) {
  return napi::GetRequired<Napi::Function>(env.Global(), "Map").FlatMap<Napi::Object>([](auto mapConstructor) {
    auto env = mapConstructor.Env();
    Napi::EscapableHandleScope scope(env);
    auto map = mapConstructor.New({});
    if (env.IsExceptionPending()) {
      return Validation<Napi::Object>::Invalid(env.GetAndClearPendingException().Message());
    }
    return Pure(scope.Escape(map).template As<Napi::Object>());
  });
}

static Maybe<Errors> SetMap(Napi::Object map, Napi::Value key, Napi::Value value) {
  return napi::GetRequired<Napi::Function>(map.Env().Global(), "Map")
  .FlatMap<Napi::Object>([](auto mapConstructor) { return napi::GetRequired<Napi::Object>(mapConstructor, "prototype"); })
  .FlatMap<Napi::Function>([](auto mapPrototype) { return napi::GetRequired<Napi::Function>(mapPrototype, "set"); })
  .FlatMap<Maybe<Errors>>([map, key, value](auto set) {
    auto env = map.Env();
    Napi::HandleScope scope(env);
    set.Call(map, { key, value });
    if (env.IsExceptionPending()) {
      return Validation<Maybe<Errors>>::Invalid(env.GetAndClearPendingException().Message());
    }
    return Pure(MakeNothing<Errors>());
  }).FromValidation([](auto errors) {
    return MakeJust(errors);
  });
}

template <typename T>
static Maybe<Errors> DoSet(Napi::Object map, std::string key, T value) {
  auto env = map.Env();
  Napi::HandleScope scope(env);
  auto maybeKey = From<Napi::Value>(std::make_pair(env, key));
  if (maybeKey.IsInvalid()) {
    return MakeJust(maybeKey.ToErrors());
  }
  auto maybeValue = From<Napi::Value>(std::make_pair(env, value));
  if (maybeValue.IsInvalid()) {
    return MakeJust(maybeValue.ToErrors());
  }
  return SetMap(map, maybeKey.UnsafeFromValid(), maybeValue.UnsafeFromValid());
}

TO_NAPI_IMPL(rtc::scoped_refptr<webrtc::RTCStatsReport>, pair) {
  return CreateMap(pair.first).FlatMap<Napi::Value>([value = pair.second](auto map) {
    auto env = map.Env();
    Napi::EscapableHandleScope scope(env);
    for (const webrtc::RTCStats& stats : *value) {
      auto result = DoSet(map, stats.id(), &stats);
      if (result.IsJust()) {
        return Validation<Napi::Value>::Invalid(result.UnsafeFromJust());
      }
    }
    return Pure(scope.Escape(map));
  });
}

}  // namespace node_webrtc
