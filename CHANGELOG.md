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
