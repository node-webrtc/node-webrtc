0.1.1
=====

Bug Fixes
---------

- Calling `createDataChannel` on a closed RTCPeerConnection no longer returns
  `undefined`; instead, it raises an InvalidStateError (#314, #382).

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
