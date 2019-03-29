#pragma once

#include <iosfwd>
#include <map>
#include <utility>
#include <vector>

#include "src/converters.h"

namespace node_webrtc {

typedef std::pair<double, std::vector<std::map<std::string, std::string>>> RTCStatsResponseInit;

DECLARE_TO_JS(RTCStatsResponseInit)

}  // namespace node_webrtc
