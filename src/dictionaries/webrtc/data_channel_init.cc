#include "src/dictionaries/webrtc/data_channel_init.h"

#include <cstdint>
#include <iosfwd>
#include <string>

#include <webrtc/api/data_channel_interface.h>

#include "src/enums/node_webrtc/rtc_priority_type.h"
#include "src/functional/maybe.h"
#include "src/functional/validation.h"

namespace node_webrtc {

#define DATA_CHANNEL_INIT_FN CreateDataChannelInit
#define DATA_CHANNEL_INIT_LIST \
  DICT_DEFAULT(bool, ordered, "ordered", true) \
  DICT_OPTIONAL(uint32_t, maxPacketLifeTime, "maxPacketLifeTime") \
  DICT_OPTIONAL(uint32_t, maxRetransmits, "maxRetransmits") \
  DICT_DEFAULT(std::string, protocol, "protocol", "") \
  DICT_DEFAULT(bool, negotiated, "negotiated", false) \
  DICT_OPTIONAL(uint32_t, id, "id") \
  DICT_DEFAULT(RTCPriorityType, priority, "priority", RTCPriorityType::kLow)

static Validation<webrtc::DataChannelInit> DATA_CHANNEL_INIT_FN(
    const bool ordered,
    const Maybe<uint32_t> maxPacketLifeTime,
    const Maybe<uint32_t> maxRetransmits,
    const std::string& protocol,
    const bool negotiated,
    const Maybe<uint32_t> id,
    const RTCPriorityType) {
  if (id.FromMaybe(0) > UINT16_MAX) {
    return Validation<webrtc::DataChannelInit>::Invalid("id must be between 0 and 65534, inclusive");
  } else if (maxPacketLifeTime.IsJust() && maxRetransmits.IsJust()) {
    return Validation<webrtc::DataChannelInit>::Invalid("You cannot set both maxPacketLifeTime and maxRetransmits");
  }
  webrtc::DataChannelInit init;
  init.ordered = ordered;
  init.maxRetransmitTime = maxPacketLifeTime.Map([](auto i) { return static_cast<int>(i); }).FromMaybe(-1);
  init.maxRetransmits = maxRetransmits.Map([](auto i) { return static_cast<int>(i); }).FromMaybe(-1);
  init.protocol = protocol;
  init.negotiated = negotiated;
  init.id = id.Map([](auto i) { return static_cast<int>(i); }).FromMaybe(-1);
  return Pure(init);
}

}  // namespace node_webrtc

#define DICT(X) DATA_CHANNEL_INIT ## X
#include "src/dictionaries/macros/impls.h"
#undef DICT
