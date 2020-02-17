Nonstandard APIs
================

MediaStream
-----------

MediaStreams in node-webrtc can be constructed with arbitrary IDs. For example,
the following MediaStream, `stream`, has its ID set to "foo".

```js
const stream = new MediaStream({ id: 'foo' });
stream.id === 'foo';  // true
```

RTCConfiguration
----------------

### `portRange`

RTCConfiguration accepts a nonstandard property, `portRange`. This property
constrains the port range used by the RTCPeerConnection's ICE transports. For
example,

```js
const { RTCPeerConnection } = require('wrtc');

const pc = new RTCPeerConnection({
  portRange: {
    min: 10000, // defaults to 0
    max: 20000  // defaults to 65535
  }
});
```

### `sdpSemantics`

RTCConfiguration accepts a nonstandard property, `sdpSemantics`. When set to
"plan-b", this property disables RTCRtpTransceivers and enables Plan B format
SDPs for a particular RTCPeerConnection. `sdpSemantics` defaults to the value
of the `SDP_SEMANTICS` environment variable. If `SDP_SEMANTICS` is unset, the
default is "unified-plan".

```js
const { RTCPeerConnection } = require('wrtc');

const pc = new RTCPeerConnection({
  sdpSemantics: 'plan-b'  // default is "unified-plan"
});
```

```
SDP_SEMANTICS=plan-b node app.js
```

Programmatic Audio
------------------

node-webrtc includes nonstandard, programmatic audio APIs in the form of
RTCAudioSource and RTCAudioSink. With these APIs, you can

 * Pass audio samples to RTCAudioSource via the `onData` method. Then use the
   RTCAudioSource's `createTrack` method to create a local audio
   MediaStreamTrack.
 * Construct an RTCAudioSink from a local or remote audio MediaStreamTrack. The
   RTCAudioSink will emit a "data" event every time audio samples are received.
   When you're finished, stop the RTCAudioSink by calling `stop`.

For example,

```js
const { RTCAudioSource, RTCAudioSink } = require('wrtc').nonstandard;

const source = new RTCAudioSource();
const track = source.createTrack();
const sink = new RTCAudioSink(track);

const sampleRate = 8000;
const samples = new Int16Array(sampleRate / 100);  // 10 ms of 16-bit mono audio

const data = {
  samples,
  sampleRate
};

const interval = setInterval(() => {
  // Update audioData in some way before sending.
  source.onData(data);
});

sink.ondata = data => {
  // Do something with the received audio samples.
};

setTimeout(() => {
  clearInterval(interval);
  track.stop();
  sink.stop();
}, 10000);
```

### RTCAudioSource

```webidl
[constructor]
interface RTCAudioSource {
  MediaStreamTrack createTrack();
  void onData(RTCAudioData data);
};

dictionary RTCAudioData {
  required Int16Array samples;
  required unsigned short sampleRate;
  octet bitsPerSample = 16;
  octet channelCount = 1;
  unsigned short numberOfFrames;
};
```

 * Calling `createTrack` will return a local audio MediaStreamTrack whose source
   is the RTCAudioSource.
 * Calling `onData` with RTCAudioData pushes a new audio samples to every
   non-stopped local audio MediaStreamTrack created with `createTrack`.
 * RTCAudioData should represent 10 ms worth of 16-bit audio samples.

### RTCAudioSink

```webidl
[constructor(MediaStreamTrack track)]
interface RTCAudioSink: EventTarget {
  void stop();
  readonly attribute boolean stopped;
  attribute EventHandler ondata;
};
```

 * RTCAudioSink's constructor accepts a local or remote audio MediaStreamTrack.
 * As long as neither the RTCAudioSink nor the RTCAudioSink's MediaStreamTrack
   are stopped, the RTCAudioSink will raise a "data" event any time
   RTCAudioData is received.
 * The "data" event has all the properties of RTCAudioData.
 * RTCAudioSink must be stopped by calling `stop`.

Programmatic Video
------------------

node-webrtc includes nonstandard, programmatic video APIs in the form of
RTCVideoSource and RTCVideoSink. With these APIs, you can

 * Pass [I420](https://wiki.videolan.org/YUV/#I420) frames to RTCVideoSource
   via the `onFrame` method. Then use RTCVideoSource's `createTrack` method to
   create a local video MediaStreamTrack.
 * Construct an RTCVideoSink from a local or remote video MediaStreamTrack. The
   RTCVideoSink will emit a "frame" event every time an I420 frame is received.
   When you're finished, stop the RTCVideoSink by calling `stop`.

For example,

```js
const { RTCVideoSource, RTCVideoSink } = require('wrtc').nonstandard;

const source = new RTCVideoSource();
const track = source.createTrack();
const sink = new RTCVideoSink(track);

const width = 320;
const height = 240;
const data = new Uint8ClampedArray(width * height * 1.5);
const frame = { width, height, data };

const interval = setInterval(() => {
  // Update the frame in some way before sending.
  source.onFrame(frame);
});

sink.onframe = ({ frame }) => {
  // Do something with the received frame.
};

setTimeout(() => {
  clearInterval(interval);
  track.stop();
  sink.stop();
}, 10000);
```

node-webrtc also includes bindings to some
[libyuv](https://chromium.googlesource.com/libyuv/libyuv/) functions for
handling I420 frames. These can be useful when converting to and from RGBA.

### RTCVideoSource

```webidl
[constructor(optional RTCVideoSourceInit init)]
interface RTCVideoSource {
  readonly attribute boolean isScreencast;
  readonly attribute boolean? needsDenoising;
  MediaStreamTrack createTrack();
  void onFrame(RTCVideoFrame frame);
};

dictionary RTCVideoSourceInit {
  boolean isScreencast = false;
  boolean needsDenoising;
};

dictionary RTCVideoFrame {
  required unsigned long width;
  required unsigned long height;
  required Uint8ClampedArray data;
  unsigned short rotation = 0;
};
```

 * Calling `createTrack` will return a local video MediaStreamTrack whose
   source is the RTCVideoSource.
 * Calling `onFrame` with an RTCVideoFrame pushes a new video frame to every
   non-stopped local video MediaStreamTrack created with `createTrack`.
 * An RTCVideoFrame represents an I420 frame.
 * RTCVideoFrame `rotation` is either 0, 90, 180, or 270.

### RTCVideoSink

```webidl
[constructor(MediaStreamTrack track)]
interface RTCVideoSink: EventTarget {
  void stop();
  readonly attribute boolean stopped;
  attribute EventHandler onframe;
};
```

 * RTCVideoSink's constructor accepts a local or remote video MediaStreamTrack.
 * As long as neither the RTCVideoSink nor the RTCVideoSink's MediaStreamTrack
   are stopped, the RTCVideoSink will raise a "frame" event any time an
   RTCVideoFrame is received.
 * The "frame" event has a property, `frame`, of type RTCVideoFrame.
 * RTCVideoSink must be stopped by calling `stop`.

### `i420ToRgba` and `rgbaToI420`

These two functions are bindings to libyuv that provide conversions between
I420 and RGBA frames. WebRTC expects I420, whereas APIs like the
[Canvas API](https://developer.mozilla.org/en-US/docs/Web/API/Canvas_API)
expect RGBA, so these functions are useful for converting between. For example,

```js
const { i420ToRgba, rgbaToI420 } = require('wrtc').nonstandard;

const width = 640;
const height = 480;
const i420Data = new Uint8ClampedArray(width * height * 1.5);
const rgbaData = new Uint8ClampedArray(width * height * 4);
const i420Frame = { width, height, data: i420Data };
const rgbaFrame = { width, height, data: rgbaData };

i420ToRgba(i420Frame, rgbaFrame);
rgbaToI420(rgbaFrame, i420Frame);
```
