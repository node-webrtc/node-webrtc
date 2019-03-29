#pragma once

namespace webrtc { struct DataChannelInit; }

#define DATA_CHANNEL_INIT webrtc::DataChannelInit

#define DICT(X) DATA_CHANNEL_INIT ## X
#include "src/dictionaries/macros/decls.h"
#undef DICT
