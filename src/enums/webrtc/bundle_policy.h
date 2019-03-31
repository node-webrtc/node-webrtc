#pragma once

#include <webrtc/api/peer_connection_interface.h>

// IWYU pragma: no_include "src/enums/macros/impls.h"

#define BUNDLE_POLICY webrtc::PeerConnectionInterface::BundlePolicy
#define BUNDLE_POLICY_NAME "RTCBundlePolicy"
#define BUNDLE_POLICY_LIST \
  ENUM_SUPPORTED(BUNDLE_POLICY::kBundlePolicyBalanced, "balanced") \
  ENUM_SUPPORTED(BUNDLE_POLICY::kBundlePolicyMaxCompat, "max-compat") \
  ENUM_SUPPORTED(BUNDLE_POLICY::kBundlePolicyMaxBundle, "max-bundle")

#define ENUM(X) BUNDLE_POLICY ## X
#include "src/enums/macros/decls.h"
#undef ENUM
