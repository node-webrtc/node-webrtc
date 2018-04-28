/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_COMMON_H_
#define SRC_COMMON_H_

#include <stdio.h>
#include <string.h>

#include "nan.h"

#define WARN(msg) fprintf(stdout, "\033[01;33m native:%s \033[00m\n", msg)
#define ERROR(msg) fprintf(stdout, "\033[01;32m native:%s \033[00m\n", msg)
#define INFO(msg) fprintf(stdout, "\033[01;34m native:%s \033[00m\n", msg)

#if defined(TRACING)

#define TRACE(msg) printf("   TRACE: %s\n", msg)
#define TRACE_S(msg, s) printf("   TRACE: %s : %s\n", msg, s)
#define TRACE_I(msg, i) printf("   TRACE: %s : %d\n", msg, i)
#define TRACE_U(msg, u) printf("   TRACE: %s : %u\n", msg, u)
#define TRACE_X(msg, x) printf("   TRACE: %s : 0x%x\n", msg, x)
#define TRACE_PTR(msg, p) printf("   TRACE: %s : %p\n", msg, p)
#define TRACE_CALL printf("-> TRACE: Call::%s\n", __PRETTY_FUNCTION__)
#define TRACE_CALL_I(p1) printf("-> TRACE: Call::%s(%d)\n", __PRETTY_FUNCTION__, p1)
#define TRACE_CALL_P(p1) printf("-> TRACE: Call::%s(%lx)\n", __PRETTY_FUNCTION__, p1)
#define TRACE_END printf("<- Call::%s\n", __PRETTY_FUNCTION__)

#else

#define TRACE(msg)
#define TRACE_S(msg, s)
#define TRACE_I(msg, i)
#define TRACE_U(msg, u)
#define TRACE_X(msg, x)
#define TRACE_PTR(msg, x)
#define TRACE_CALL
#define TRACE_CALL_I(p1)
#define TRACE_CALL_P(p1)
#define TRACE_END

#endif

#endif  // SRC_COMMON_H_
