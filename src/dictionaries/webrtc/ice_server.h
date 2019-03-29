#pragma once

#include <webrtc/api/peer_connection_interface.h>

#define ICE_SERVER webrtc::PeerConnectionInterface::IceServer

#define DICT(X) ICE_SERVER ## X
#include "src/dictionaries/macros/decls.h"
#undef DICT
