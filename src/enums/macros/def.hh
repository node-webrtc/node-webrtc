#ifdef ENUM

namespace node_webrtc {

#define ENUM_SUPPORTED(VALUE, STRING) VALUE,
#define ENUM_UNSUPPORTED(VALUE, STRING, ERROR) VALUE,

enum ENUM() { ENUM(_LIST) };

#undef ENUM_SUPPORTED
#undef ENUM_UNSUPPORTED

} // namespace node_webrtc

#endif // ENUM
