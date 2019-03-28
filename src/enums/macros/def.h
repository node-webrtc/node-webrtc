#ifdef ENUM

namespace node_webrtc {

#define ENUM_IMPL_VALUE(VALUE) VALUE,
#define SUPPORTED(ENUM, VALUE, STRING) ENUM_IMPL_VALUE(VALUE)
#define UNSUPPORTED(ENUM, VALUE, STRING, ERROR) ENUM_IMPL_VALUE(VALUE)

enum ENUM() {
  ENUM(_LIST)
};

#undef ENUM_IMPL_VALUE
#undef SUPPORTED
#undef UNSUPPORTED

}  // namespace webrtc

#endif  // ENUM
