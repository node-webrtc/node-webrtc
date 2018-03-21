0.0.68
======

New Features
------------

- Expanded RTCConfiguration support. We now support `iceTransportPolicy`,
  `bundlePolicy`, `rtcpMuxPolicy`, and `iceCandidatePoolSize`.
- Added support for `getConfiguration` and `setConfiguration`.
- Added support for a non-standard RTCConfiguration attribute, `portRange`. You
  can use this to constrain the `min` and `max` ports to be used when creating
  ICE transports. For example,

  ```js
  const { RTCPeerConnection } = require('wrtc');
  const pc = new RTCPeerConnection({ portRange: { min: 0, max: 65535 } });
  ```

  `portRange` cannot be updated by `setConfiguration`.

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
