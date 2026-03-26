#pragma once

#include <cstdint>
#include <string>

#define RTC_PEER_CONNECTION_ICE_ERROR_EVENT RTCPeerConnectionIceErrorEvent
#define RTC_PEER_CONNECTION_ICE_ERROR_EVENT_LIST                               \
  DICT_REQUIRED(std::string, address, "address")                               \
  DICT_REQUIRED(int, port, "port")                                             \
  DICT_REQUIRED(std::string, url, "url")                                       \
  DICT_REQUIRED(uint8_t, errorCode, "errorCoode")                              \
  DICT_REQUIRED(std::string, errorText, "errorText")

#define DICT(X) RTC_PEER_CONNECTION_ICE_ERROR_EVENT##X
#include "src/dictionaries/macros/def.hh"
// ordering
#include "src/dictionaries/macros/decls.hh"
#undef DICT
