#include "src/dictionaries/node_webrtc/rtc_outh_credential.h"

#include "src/functional/validation.h"

namespace node_webrtc {

#define RTC_OAUTH_CREDENTIAL_FN CreateRTCOAuthCredential

static Validation<RTC_OAUTH_CREDENTIAL> RTC_OAUTH_CREDENTIAL_FN(
    const std::string& macKey,
    const std::string& accessToken) {
  return Pure<RTC_OAUTH_CREDENTIAL>({macKey, accessToken});
}

}  // namespace node_webrtc

#define DICT(X) RTC_OAUTH_CREDENTIAL ## X
#include "src/dictionaries/macros/impls.h"
#undef DICT
