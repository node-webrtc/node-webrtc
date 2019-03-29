#pragma once

#include <memory>

namespace webrtc { class IceCandidateInterface; }

#define ICE_CANDIDATE_INTERFACE webrtc::IceCandidateInterface*

#define DICT(X) ICE_CANDIDATE_INTERFACE ## X
#include "src/dictionaries/macros/decls.h"
#undef DICT

namespace node_webrtc {

DECLARE_FROM_JS(std::shared_ptr<webrtc::IceCandidateInterface>)

}  // namespace node_webrtc
