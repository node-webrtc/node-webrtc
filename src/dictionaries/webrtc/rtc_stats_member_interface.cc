#include "src/dictionaries/webrtc/rtc_stats_member_interface.h"

#include <cstdint>
#include <iosfwd>
#include <string>
#include <utility>
#include <vector>

#include <node-addon-api/napi.h>
#include <webrtc/api/stats/rtc_stats.h>

#include "src/converters.h"

namespace node_webrtc {

TO_NAPI_IMPL(const webrtc::RTCStatsMemberInterface*, pair) {
  auto env = pair.first;
  auto value = pair.second;
  switch (value->type()) {
    case webrtc::RTCStatsMemberInterface::Type::kBool:  // bool
      return From<Napi::Value>(std::make_pair(env, *value->cast_to<webrtc::RTCStatsMember<bool>>()));
    case webrtc::RTCStatsMemberInterface::Type::kInt32:  // int32_t
      return From<Napi::Value>(std::make_pair(env, *value->cast_to<webrtc::RTCStatsMember<int32_t>>()));
    case webrtc::RTCStatsMemberInterface::Type::kUint32:  // uint32_t
      return From<Napi::Value>(std::make_pair(env, *value->cast_to<webrtc::RTCStatsMember<uint32_t>>()));
    case webrtc::RTCStatsMemberInterface::Type::kInt64:   // int64_t
      return From<Napi::Value>(std::make_pair(env, *value->cast_to<webrtc::RTCStatsMember<int64_t>>()));
    case webrtc::RTCStatsMemberInterface::Type::kUint64:  // uint64_t
      return From<Napi::Value>(std::make_pair(env, *value->cast_to<webrtc::RTCStatsMember<uint64_t>>()));
    case webrtc::RTCStatsMemberInterface::Type::kDouble:  // double
      return From<Napi::Value>(std::make_pair(env, *value->cast_to<webrtc::RTCStatsMember<double>>()));
    case webrtc::RTCStatsMemberInterface::Type::kString:  // std::string
      return From<Napi::Value>(std::make_pair(env, *value->cast_to<webrtc::RTCStatsMember<std::string>>()));
    case webrtc::RTCStatsMemberInterface::Type::kSequenceBool:  // std::vector<bool>
      return From<Napi::Value>(std::make_pair(env, *value->cast_to<webrtc::RTCStatsMember<std::vector<bool>>>()));
    case webrtc::RTCStatsMemberInterface::Type::kSequenceInt32:  // std::vector<int32_t>
      return From<Napi::Value>(std::make_pair(env, *value->cast_to<webrtc::RTCStatsMember<std::vector<int32_t>>>()));
    case webrtc::RTCStatsMemberInterface::Type::kSequenceUint32:  // std::vector<uint32_t>
      return From<Napi::Value>(std::make_pair(env, *value->cast_to<webrtc::RTCStatsMember<std::vector<uint32_t>>>()));
    case webrtc::RTCStatsMemberInterface::Type::kSequenceInt64:  // std::vector<int64_t>
      return From<Napi::Value>(std::make_pair(env, *value->cast_to<webrtc::RTCStatsMember<std::vector<int64_t>>>()));
    case webrtc::RTCStatsMemberInterface::Type::kSequenceUint64:  // std::vector<uint64_t>
      return From<Napi::Value>(std::make_pair(env, *value->cast_to<webrtc::RTCStatsMember<std::vector<uint64_t>>>()));
    case webrtc::RTCStatsMemberInterface::Type::kSequenceDouble:  // std::vector<double>
      return From<Napi::Value>(std::make_pair(env, *value->cast_to<webrtc::RTCStatsMember<std::vector<double>>>()));
    case webrtc::RTCStatsMemberInterface::Type::kSequenceString:  // std::vector<std::string>
      return From<Napi::Value>(std::make_pair(env, *value->cast_to<webrtc::RTCStatsMember<std::vector<std::string>>>()));
  }
}

} // namespace node_webrtc
