/*
 *  Copyright 2018 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "src/webrtc/task_queue_for_test.h"

namespace node_webrtc {

TaskQueueForTest::TaskQueueForTest(const char* queue_name, Priority priority)
  : TaskQueue(queue_name, priority) {}
TaskQueueForTest::~TaskQueueForTest() {}

}  // namespace node_webrtc
