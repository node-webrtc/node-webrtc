#include "src/dictionaries/node_webrtc/rtc_dtls_fingerprint.h"

#include "src/functional/maybe.h"  // IWYU pragma: keep

namespace node_webrtc {

#define RTC_DTLS_FINGERPRINT_FN CreateRTCDtlsFingerprint

static RTC_DTLS_FINGERPRINT RTC_DTLS_FINGERPRINT_FN(
    const Maybe<std::string>& algorithm,
    const Maybe<std::string>& value) {
  return {algorithm, value};
}

}  // namespace node_webrtc

#define DICT(X) RTC_DTLS_FINGERPRINT ## X
#include "src/dictionaries/macros/impls.h"
#undef DICT
