/* Copyright (c) 2023 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */

export interface RTCVideoFrame {
  width: number;
  height: number;
  data: Uint8Array;
}

export const i420ToRgba: (i420: RTCVideoFrame, rgba: RTCVideoFrame) => void;
export const rgbaToI420: (rgba: RTCVideoFrame, i420: RTCVideoFrame) => void;

export interface RTCAudioSink extends EventTarget {
  stop(): void;
  readonly stopped: boolean;
  ondata: EventHandler;
};

export const RTCAudioSink: {
  prototype: RTCAudioSink;
  new (track: MediaStreamTrack): RTCAudioSink;
}

export interface RTCAudioData {
  samples: Int16Array;
  sampleRate: number;
  bitsPerSample?: number; // default = 16
  channelCount?: number; // default = 1
  numberOfFrames?: number; // default = 10ms of audio at the given sampleRate
}

export interface RTCAudioSource {
  createTrack(): MediaStreamTrack;
  onData(data: RTCAudioData): void;
}

export const RTCAudioSource: {
  prototype: RTCAudioSource;
  new (): RTCAudioSource;
}

export interface RTCVideoSink extends EventTarget {
  stop(): void;
  readonly stopped: boolean;
  onframe: EventHandler;
};

export const RTCVideoSink: {
  prototype: RTCVideoSink;
  new (track: MediaStreamTrack): RTCVideoSink;
}

export interface RTCVideoData {
  samples: Int16Array;
  sampleRate: number;
  bitsPerSample?: number; // default = 16
  channelCount?: number; // default = 1
  numberOfFrames?: number; // default = 10ms of audio at the given sampleRate
}

export interface RTCVideoSourceInit {
  isScreencast?: boolean; // default = false
  needsDenoising?: boolean;
}

export interface RTCVideoSource {
  readonly isScreencast: boolean;
  readonly needsDenoising?: boolean;
  createTrack(): MediaStreamTrack;
  onFrame(data: RTCVideoFrame): void;
}

export const RTCVideoSource: {
  prototype: RTCVideoSource;
  new (init?: RTCVideoSourceInit): RTCVideoSource;
}
