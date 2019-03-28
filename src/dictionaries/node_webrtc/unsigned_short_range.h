#pragma once

#include <cstdint>  // IWYU pragma: keep

// IWYU pragma: no_forward_declare node_webrtc::UnsignedShortRange
// IWYU pragma: no_include <_types/_uint16_t.h>

#define UNSIGNED_SHORT_RANGE UnsignedShortRange
#define UNSIGNED_SHORT_RANGE_NAME "UnsignedShortRange"
#define UNSIGNED_SHORT_RANGE_LIST \
  OPTIONAL(uint16_t, min, "min") \
  OPTIONAL(uint16_t, max, "max")

#define DICT(X) UNSIGNED_SHORT_RANGE ## X
#include "src/dictionaries/macros/def.h"
#include "src/dictionaries/macros/decls.h"
#undef DICT
