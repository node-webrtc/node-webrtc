/* Copyright (c) 2023 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */

// Re-export most types to have the same signature as builtin ones
// (the signatures actually are slightly different, but we are meant to be
// a polyfill library so we'll ignore that for now)
export {
  MediaStream,
  MediaStreamTrack,
  RTCDataChannel,
  RTCDataChannelEvent,
  RTCDtlsTransport,
  RTCIceCandidate,
  RTCIceTransport,
  RTCPeerConnection,
  RTCPeerConnectionIceEvent,
  RTCRtpReceiver,
  RTCRtpSender,
  RTCRtpTransceiver,
  RTCSctpTransport,
  RTCSessionDescription,
  getUserMedia,
  mediaDevices,
};

// Include the non-standard APIs too
export * as nonstandard from "./nonstandard";
