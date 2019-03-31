#ifdef ENUM

namespace node_webrtc {

#define ENUM_SUPPORTED(ENUM, VALUE, STRING) VALUE,
#define ENUM_UNSUPPORTED(ENUM, VALUE, STRING, ERROR) VALUE,

enum ENUM() {
  ENUM(_LIST)
};

#undef ENUM_SUPPORTED
#undef ENUM_UNSUPPORTED

}  // namespace webrtc

#endif  // ENUM
