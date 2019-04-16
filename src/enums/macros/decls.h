#ifdef ENUM

#include "src/converters.h"
#include "src/converters/napi.h"

namespace node_webrtc {

DECLARE_CONVERTER(ENUM(), std::string)
DECLARE_CONVERTER(std::string, ENUM())
DECLARE_TO_AND_FROM_NAPI(ENUM())

}  // namespace node_webrtc

#endif  // ENUM
