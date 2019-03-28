#include "src/dictionaries/node_webrtc/rtc_outh_credential.h"

namespace node_webrtc {

#define RTC_OAUTH_CREDENTIAL_FN CreateRTCOAuthCredential

static RTC_OAUTH_CREDENTIAL RTC_OAUTH_CREDENTIAL_FN(
    const std::string& macKey,
    const std::string& accessToken) {
  return {macKey, accessToken};
}

}  // namespace node_webrtc

#define DICT(X) RTC_OAUTH_CREDENTIAL ## X
#include "src/dictionaries/macros/impls.h"
#undef DICT
