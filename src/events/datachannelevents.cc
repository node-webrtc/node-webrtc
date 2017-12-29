/* Copyright (c) 2017 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/events/datachannelevents.h"

#include "src/datachannel.h"

using node_webrtc::DataChannel;
using node_webrtc::DataChannelStateChangeEvent;
using node_webrtc::ErrorEvent;
using node_webrtc::MessageEvent;

void DataChannelStateChangeEvent::Dispatch(DataChannel& dataChannel) {
  dataChannel.HandleStateEvent(*this);
}

namespace node_webrtc {
template<>
void ErrorEvent<DataChannel>::Dispatch(DataChannel &dataChannel) {
  dataChannel.HandleErrorEvent(*this);
}
}

void MessageEvent::Dispatch(DataChannel& dataChannel) {
  dataChannel.HandleMessageEvent(*this);
}
