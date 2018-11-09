0.3.2
=====

New Features
------------

- Support for Node 11 on Windows.

0.3.1
=====

This release adds a number of new features and brings us closer to
spec-compliance, thanks to the tests at
[web-platform-tests/wpt](http://github.com/web-platform-tests/wpt).

New Features
------------

### `getUserMedia`

This release adds limited `getUserMedia` support. You can create audio and video
MediaStreamTracks; however, the resulting MediaStreamTracks do not capture any
media. You can add these MediaStreamTracks to an RTCPeerConnection; however, no
media will be transmitted. You can confirm by checking `bytesSent` and
`bytesReceived` in `getStats`.

```js
const { getUserMedia } = require('wrtc');

getUserMedia({
  audio: true,
  video: true
}).then(stream => {
  stream.getTracks().forEach(track => stop());
});
```

Although we will parse and validate some members of the MediaStreamConstraints
and related dictionaries, we do not use their values at this time.

### `getStats`

This release adds limited standards-compliant `getStats` support. Previous
node-webrtc releases exposed the legacy, callback-based `getStats` API. This
release preserves that API but adds the Promise-based API. Neither the
MediaStreamTrack selector argument nor the RTCRtpSender- and
RTCRtpReceiver-level `getStats` APIs are implemented at this time.

```js
// Legacy API
pc.getStats(
  response => { /* ... */ },
  console.error
);

// Standards-compliant API
pc.getStats().then(
  report => { /* ... */ },
  console.error
);
```

### Unified Plan and `sdpSemantics`

This release adds support for RTCRtpTransceivers and Unified Plan SDP via

* A non-standard RTCConfiguration option, `sdpSemantics`, and
* An environment variable, `SDP_SEMANTICS`.

Construct an RTCPeerConnection with `sdpSemantics` set to "unified-plan" or
launch your application with `SDP_SEMANTICS=unified-plan` to enable
RTCRtpTransceiver support; otherwise, "plan-b" is the default.

```js
const { RTCPeerConnection } = require('wrtc');

const pc = new RTCPeerConnection({
  sdpSemantics: 'unified-plan'  // default is "plan-b"
});
```

```
SDP_SEMANTICS=unified-plan node app.js
```

### RTCRtpTransceiver

You can use RTCRtpTransceivers and related APIs when Unified Plan is enabled.
This includes the following RTCPeerConnection methods

- `addTransceiver`
- `getTransceivers`

and the following RTCTrackEvent properties

- `transceiver`

The following RTCRtpTransceiver methods are supported

- `stop`

as well as the following RTCRtpTransceiver properties

- `mid`
- `sender`
- `receiver`
- `stopped`
- `direction`
- `currentDirection`

`setCodecPreferences` is not yet implemented. When calling `addTransceiver`,
only the following RTCRtpTransceiverInit dictionary members are supported

- `direction`
- `streams`

```js
const assert = require('assert');
const { MediaStream, RTCPeerConnection, RTCRtpTransceiver } = require('wrtc');

const pc = new RTCPeerConnection({
  sdpSemantics: 'unified-plan'
});

const t1 = pc.addTransceiver('audio', {
  direction: 'recvonly'
});

const t2 = pc.addTransceiver(t1.receiver.track, {
  direction: 'sendonly',
  streams: [new MediaStream()]
});
```

### MediaStreamTrack

Added limited support for the `muted` property (it always returns `false`).

### Miscellaneous

- APIs that should throw DOMExceptions, such as `addTrack`, will use
  [domexception](https://github.com/jsdom/domexception) to construct those
  DOMExceptions, if installed.

Bug Fixes
---------

- Calling `addTrack` twice with the same MediaStreamTrack should throw an
  InvalidAccessError (#442).
- MediaStream's `getTrackById` did not work for video MediaStreamTracks.
- MediaStream's `clone` method did not `clone` MediaStreamTracks.
- MediaStreamTrack's `readyState` was not updated when `stop` was called.

0.3.0
=====

New Features
------------

- Support for Node 11. Binaries are available for Linux and macOS. Windows
  binaries will become available in a subsequent release once AppVeyor gains
  support for Node 11.
- Updated to WebRTC M70. This release no longer uses
  [mayeut/libwebrtc](https://github.com/mayeut/libwebrtc); instead, WebRTC is
  built from source.

Breaking Changes
----------------

- Dropped support for Node 9
- Minimum CMake version bumped to 3.12
- Minimum GCC version bumped to 5.4
- Minimum Microsoft Visual Studio version bumped to 2017

Bug Fixes
---------

- Updating to WebRTC M70 fixes an RTCDataChannel-related interop bug with recent
  Firefox releases (#444).

0.2.2
=====

Bug Fixes
---------

- Destroy AudioDeviceModule on the worker thread.

0.2.1
=====

Bug Fixes
---------

- Fixed an AudioDeviceModule memory and thread leak (#429).
- Fixed an issue where closing an RTCPeerConnection would raise "open" events on
  any RTCDataChannels whose `readyState` was "connecting" (#436).

0.2.0
=====

Breaking Changes
----------------

- Dropped support for Node 4, 5 and 7 (#408).

Bug Fixes
---------

- Fixed a race when closing an RTCDataChannel (#358).
- Fixed memory leaks in `createOffer`, `createAnswer`, `addIceCandidate`, and
  `getStats` (#425).

0.1.6
=====

Bug Fixes
---------

- Fixed an issue with receiving multiple ArrayBuffers over an RTCDataChannel
  that could cause invalid memory accesses (#406).

0.1.5
=====

New Features
------------

- This release allows relaying remote MediaStreamTracks. This can be useful for
  test applications. **Note:** currently, Windows cannot relay audio
  MediaStreamTracks, only video.

### MediaStream

This release adds support for MediaStream. Most MediaStream APIs are supported,
excluding the "addtrack" and "removetrack" events. You can construct
MediaStreams as follows:

```js
const { MediaStream } = require('wrtc');

const stream1 = new MediaStream();
const stream2 = new MediaStream(stream1);
```

Assuming you already have some Array of MediaStreamTracks, `tracks`, you can
also construct a MediaStream with

```js
const stream3 = new MediaStream(tracks);
```

Or, if you have an existing MediaStream, you can `clone` it.

```js
const stream4 = stream3.clone();
```

This release also adds support for the following methods

- `getTracks`
- `getAudioTracks`
- `getVideoTracks`
- `getTrackById`
- `addTrack`
- `removeTrack`

and the following attributes

- `id`
- `active`

### RTCTrackEvent

This release adds support for the `streams` property.

### RTCPeerConnection

This release adds support for `addTrack`, `removeTrack`, and `getSenders`.
Although we don't yet provide a way to construct local MediaStreamTracks, you
can relay remote MediaStreamTracks as follows:

```js
pc.ontrack = ({ track, streams }) => {
  pc.addTrack(track, ...streams);
};
```

### RTCRtpSender

This release adds support for the following methods

- `getCapabilities` (always throws for now)
- `getParameters`
- `setParameters` (always returns a rejected Promise for now)
- `getStats` (always returns a rejected Promise for now)
- `replaceTrack`

and the following attributes

- `track`
- `transport` (always returns `null` for now)
- `rtcpTransport` (always returns `null` for now)

0.1.4
=====

New Features
------------

- Added support for Node 10 (#402)

### RTCPeerConnection

- Added support for `getReceivers`.
- Added partial support for the RTCTrackEvent. RTCPeerConnection will emit the
  RTCTrackEvent with two attributes: `track` and `receiver`. In the future, we
  will add support for the `streams` and `transceiver` attributes.

### RTCRtpReceiver

This release adds partial support for RTCRtpReceiver, including methods

- `getParameters`
- `getContributingSources` (always returns an empty array for now)
- `getSynchronizationSources` (always returns an empty array for now)
- `getStats` (always returns a rejected Promise for now)

and attributes

- `track`
- `transport` (always returns `null` for now)
- `rtcpTransport` (always returns `null` for now)

RTCRtpReceiver also includes partial support for the static method
`getCapabilities`; however, it always returns a rejected Promise.

### MediaStreamTrack

This release adds partial support for remote MediaStreamTracks, including
attributes

- `enabled` (read-only for now)
- `id`
- `kind`
- `readyState`

0.1.3
=====

Bug Fixes
---------

- Fixed memory leaks related to RTCPeerConnection events.

0.1.2
=====

Bug Fixes
---------

- Fixed memory leaks related to sending and receiving messages over
  RTCDataChannels (#205, #304, #319). There are some less severe leaks related
  to RTCPeerConnection events that remain. These will be addressed in a future
  release.

0.1.1
=====

Bug Fixes
---------

- Calling `createDataChannel` on a closed RTCPeerConnection no longer returns
  `undefined`; instead, it raises an InvalidStateError (#314, #382).
- Worked around WebRTC [Issue 7585](https://bugs.chromium.org/p/webrtc/issues/detail?id=7585)
  on Linux by backporting the `epoll`-based PhysicalSocketServer from WebRTC
  M61 into node-webrtc. This allows many more concurrent RTCPeerConnections on
  Linux (for example, up to 3000 in my tests, not exceeding thread limits).
  (#362)

0.1.0
=====

This project will begin to follow [SemVer](https://semver.org) in preparation
for a 1.0.0 release.

New Features
------------

Besides updating to WebRTC M60 (using
[mayeut/libwebrtc](https://github.com/mayeut/libwebrtc)), this release adds a
number of features that bring node-webrtc closer to standards-compliance. We
still have a ways to go, but we're now testing against
[w3c/web-platform-tests](https://github.com/w3c/web-platform-tests).

### RTCConfiguration

RTCPeerConnection's constructor now accepts the following standard properties:

* `bundlePolicy`
* `iceCandidatePoolSize`
* `iceServers` (no support for OAuth yet)
* `iceTransportPolicy`
* `rtcpMuxPolicy`

RTCConfiguration also accepts a non-standard property, `portRange`. This
property constrains the port range used by the RTCPeerConnection's ICE
transports. For example,

```js
const { RTCPeerConnection } = require('wrtc');

const pc = new RTCPeerConnection({
  portRange: {
    min: 10000, // defaults to 0
    max: 20000  // defaults to 65535
  }
});
```

### RTCPeerConnection

RTCPeerConnection now supports two new methods:

* `getConfiguration`
* `setConfiguration`

RTCPeerConnection now supports the following properties:

* `canTrickleIceCandidates` (always returns `null` for now)
* `connectionState` (derived from `iceConnectionState`)
* `currentLocalDescription`
* `currentRemoteDescription`
* `pendingLocalDescription`
* `pendingRemoteDescription`

RTCPeerConnection now supports the following events:

* "connectionstatechange"
* "negotiationneeded"

### RTCOfferOptions and RTCAnswerOptions

RTCPeerConnection's `createOffer` method now accepts RTCOfferOptions, and
RTCPeerConnection's `createAnswer` method now accepts RTCAnswerOptions.
RTCOfferOptions supports

* `iceRestart`
* `offerToReceiveAudio`
* `offerToReceiveVideo`

Both RTCOfferOptions and RTCAnswerOptions support `voiceActivityDetection`.

### RTCDataChannelInit

RTCPeerConnection's `createDataChannel` method now accepts

* `id`
* `maxPacketLifeTime`
* `maxRetransmits`
* `negotiated`
* `ordered`
* `protocol`

### RTCDataChannel

RTCDataChannel supports the following properties:

* `id`
* `maxRetransmits`
* `ordered`
* `priority` (always returns "high")
* `protocol`

RTCDataChannel's `send` method now supports sending Blobs provided by
[jsdom](https://github.com/jsdom/jsdom); however, there is no support for
receiving Blobs.

### Top-level Exports

Added top-level exports for

* RTCDataChannel
* RTCDataChannelEvent
* RTCPeerConnectionIceEvent

Bug Fixes
---------

- Fixed a failed assertion when closing RTCPeerConnection's or RTCDataChannel's
  event loop (#376).
- Moved AudioDeviceModule construction to the worker thread. This fixes a thread
  checker assertion raised by debug builds of libwebrtc.
- Copy StatsReports on the signaling thread. This fixes a thread checker
  assertion raised by debug builds of libwebrtc.

Breaking Changes
----------------

- Dropped support for "moz"- and "webkit"-prefixed WebRTC APIs in
  `lib/browser.js`. This means that, when bundling JavaScript that depends on
  node-webrtc for the browser, the resulting bundle will no longer depend on
  these APIs (#369).
- Dropped support for the now non-standard `url` attribute in RTCIceServer.
- Dropped support for the `RTCIceConnectionStates`, `RTCIceGatheringStates`, and
  `RTCSignalingStates` properties on the RTCPeerConnection prototype. These were
  an implementation detail that some libraries used for detecting node-webrtc.
- Dropped support for the `RTCDataStates` and `BinaryTypes` properties on the
  RTCDataChannel prototype. This, too, was an implementation detail.

0.0.67
======

Bug Fixes
---------

- ObjectWrap instances accessed in an event loop (like PeerConnection and
  DataChannel) were getting freed before the event loop completed, which caused
  segfaults. Now, we call `Ref` in the ObjectWrap instances' constructors and
  `Unref` in their event loops.
- Fixed another source of segfaults, where, if a DataChannel's
  PeerConnectionFactory was freed, accessing the underlying DataChannelInterface
  would try to use Threads which had been freed.

0.0.66
======

Bug Fixes
---------

- Fixed a CPU regression introduced in 0.0.63. We now share a single
  PeerConnectionFactoryInterface across PeerConnectionInterfaces, and we now use
  a "dummy" AudioDeviceModule instead of FakeAudioDeviceModule.

0.0.65
======

New Features
------------

- Added support for sending Buffers (#103)

Bug Fixes
---------

- Sending an ArrayBufferView over an RTCDataChannel did not take into account
  the ArrayBufferView's `offset` or `length` properties. This resulted in
  sending the entire backing ArrayBuffer instead of just the data in the
  ArrayBufferView.
- unzip-stream 0.2.2 breaks compatibility with Node 4 and 5. This release pins
  to unzip-stream 0.2.1.

Breaking Changes
----------------

- Building from source requires CMake 3.1 or newer

0.0.64
======

Bug Fixes
---------

- We no longer `Externalize` ArrayBuffers. This fixes an error when sending
  ArrayBuffers mutliple times (#262 and #264) and a memory leak (#304).
- Fixed RTCDataChannel-related segfaults by checking for `nullptr` (#236 and
  #325)

0.0.63
======

New Features
------------

- Support for Node 9
- Updated to WebRTC M57 (using [libwebrtc](https://github.com/aisouard/libwebrtc))

Breaking Changes
----------------

- Minimum Mac OS X version bumped to 10.9
- Minimum Microsoft Visual Studio version bumped to 2015
