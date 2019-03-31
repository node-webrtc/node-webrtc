#pragma once

// IWYU pragma: no_include "src/enums/macros/impls.h"

#define RTC_ICE_CREDENTIAL_TYPE RTCIceCredentialType
#define RTC_ICE_CREDENTIAL_TYPE_NAME "RTCIceCredentialType"
#define RTC_ICE_CREDENTIAL_TYPE_LIST \
  ENUM_SUPPORTED(RTC_ICE_CREDENTIAL_TYPE, kPassword, "password") \
  ENUM_SUPPORTED(RTC_ICE_CREDENTIAL_TYPE, kOAuth, "oauth")

#define ENUM(X) RTC_ICE_CREDENTIAL_TYPE ## X
#include "src/enums/macros/def.h"
#include "src/enums/macros/decls.h"
#undef ENUM
