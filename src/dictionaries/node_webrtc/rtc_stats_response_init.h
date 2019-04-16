#pragma once

#include <iosfwd>
#include <map>
#include <utility>
#include <vector>

#include "src/converters/napi.h"

namespace node_webrtc {

typedef std::pair<double, std::vector<std::map<std::string, std::string>>> RTCStatsResponseInit;

DECLARE_TO_NAPI(RTCStatsResponseInit)

}  // namespace node_webrtc
