#ifdef ENUM

namespace node_webrtc {

#define SUPPORTED(ENUM, VALUE, STRING) VALUE,
#define UNSUPPORTED(ENUM, VALUE, STRING, ERROR) VALUE,

enum ENUM() {
  ENUM(_LIST)
};

#undef SUPPORTED
#undef UNSUPPORTED

}  // namespace webrtc

#endif  // ENUM
