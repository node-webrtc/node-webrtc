#pragma once

#include <webrtc/api/peer_connection_interface.h>

// IWYU pragma: no_include "src/enums/macros/impls.h"

#define BUNDLE_POLICY webrtc::PeerConnectionInterface::BundlePolicy
#define BUNDLE_POLICY_NAME "RTCBundlePolicy"
#define BUNDLE_POLICY_LIST \
  SUPPORTED(BUNDLE_POLICY, kBundlePolicyBalanced, "balanced") \
  SUPPORTED(BUNDLE_POLICY, kBundlePolicyMaxCompat, "max-compat") \
  SUPPORTED(BUNDLE_POLICY, kBundlePolicyMaxBundle, "max-bundle")

#define ENUM(X) BUNDLE_POLICY ## X
#include "src/enums/macros/decls.h"  // IWYU pragma: keep
#undef ENUM
