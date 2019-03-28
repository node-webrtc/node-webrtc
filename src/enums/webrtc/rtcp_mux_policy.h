#pragma once

#include <webrtc/api/peer_connection_interface.h>

// IWYU pragma: no_include "src/enums/macros/impls.h"

#define RTCP_MUX_POLICY webrtc::PeerConnectionInterface::RtcpMuxPolicy
#define RTCP_MUX_POLICY_NAME "RTCRtcpMuxPolicy"
#define RTCP_MUX_POLICY_LIST \
  SUPPORTED(RTCP_MUX_POLICY, kRtcpMuxPolicyNegotiate, "negotiate") \
  SUPPORTED(RTCP_MUX_POLICY, kRtcpMuxPolicyRequire, "require")

#define ENUM(X) RTCP_MUX_POLICY ## X
#include "src/enums/macros/decls.h"  // IWYU pragma: keep
#undef ENUM
