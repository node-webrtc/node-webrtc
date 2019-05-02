#pragma once

#include <webrtc/p2p/base/transport_description.h>

// IWYU pragma: no_include "src/enums/macros/impls.h"

#define ICE_ROLE cricket::IceRole
#define ICE_ROLE_NAME "RTCIceRole"
#define ICE_ROLE_LIST \
  ENUM_SUPPORTED(ICE_ROLE::ICEROLE_CONTROLLING, "controlling") \
  ENUM_SUPPORTED(ICE_ROLE::ICEROLE_CONTROLLED, "controlled") \
  ENUM_UNSUPPORTED(ICE_ROLE::ICEROLE_UNKNOWN, "unknown", "\"unknown\" is not a valid RTCIceRole")

#define ENUM(X) ICE_ROLE ## X
#include "src/enums/macros/decls.h"
#undef ENUM
