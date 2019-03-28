#pragma once

#include <iosfwd>
#include <string>

// IWYU pragma: no_forward_declare node_webrtc::RTCDtlsFingerprint
// IWYU pragma: no_include "src/dictionaries/macros/impls.h"

#define RTC_DTLS_FINGERPRINT RTCDtlsFingerprint
#define RTC_DTLS_FINGERPRINT_NAME "RTCDtlsFingerprint"
#define RTC_DTLS_FINGERPRINT_LIST \
  OPTIONAL(std::string, algorithm, "algorithm") \
  OPTIONAL(std::string, value, "value")

#define DICT(X) RTC_DTLS_FINGERPRINT ## X
#include "src/dictionaries/macros/def.h"
#include "src/dictionaries/macros/decls.h"
#undef DICT
