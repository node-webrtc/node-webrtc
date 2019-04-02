#ifdef DICT

#include "src/converters.h"
#include "src/converters/napi.h"
#include "src/converters/v8.h"

namespace node_webrtc {

DECLARE_TO_AND_FROM_JS(DICT())
DECLARE_TO_AND_FROM_NAPI(DICT())

}  // namespace node_webrtc

#endif  // DICT
