#pragma once

#include <cstdint>

// IWYU pragma: no_forward_declare node_webrtc::RTCPeerConnectionIceErrorEvent

#define RTC_PEER_CONNECTION_ICE_ERROR_EVENT RTCPeerConnectionIceErrorEvent
#define RTC_PEER_CONNECTION_ICE_ERROR_EVENT_LIST \
  DICT_REQUIRED(std::string, hostCandidate, "hostCandidate") \
  DICT_REQUIRED(std::string, url, "url") \
  DICT_REQUIRED(uint8_t, errorCode, "errorCoode") \
  DICT_REQUIRED(std::string, errorText, "errorText")

#define DICT(X) RTC_PEER_CONNECTION_ICE_ERROR_EVENT ## X
#include "src/dictionaries/macros/def.h"
#include "src/dictionaries/macros/decls.h"
#undef DICT
