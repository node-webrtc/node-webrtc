#pragma once

#include <webrtc/api/media_types.h>

// IWYU pragma: no_include "src/enums/macros/impls.h"

// FIXME(mroberts): I'm not sure that "data" should be valid.
#define MEDIA_TYPE cricket::MediaType
#define MEDIA_TYPE_NAME "kind"
#define MEDIA_TYPE_LIST \
  ENUM_SUPPORTED(MEDIA_TYPE::MEDIA_TYPE_AUDIO, "audio") \
  ENUM_SUPPORTED(MEDIA_TYPE::MEDIA_TYPE_VIDEO, "video") \
  ENUM_SUPPORTED(MEDIA_TYPE::MEDIA_TYPE_DATA, "data")

#define ENUM(X) MEDIA_TYPE ## X
#include "src/enums/macros/decls.h"
#undef ENUM
