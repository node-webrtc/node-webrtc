[![NPM](https://nodei.co/npm/wrtc.png?downloads=true&stars=true)](https://nodei.co/npm/wrtc/)

# Contributing

The best way to get started is to read through the `Getting Started` and `Example` sections before having a look through the open [issues](https://github.com/modeswitch/node-webrtc/issues). Some of the issues are marked as `good first bug`, but feel free to contribute to any of the issues there, or open a new one if the thing you want to work on isn't there yet.

Once you've done some hacking and you'd like to have your work merged, you'll need to make a pull request. If your patch includes code, make sure to check that all the unit tests pass, including any new tests you wrote. Finally, make sure you add yourself to the `AUTHORS` file.

# Getting Started

## Prerequisites

**Important**: Make sure you have all the prerequisite libraries installed for your platform, as described [here](http://www.webrtc.org/reference/getting-started/prerequisite-sw).

We have experience recently that they have been included in latest revisions of the libWebrtc library some dev dependencies not documented on the link below. For Debian/Ubuntu/LinuxMint based systems, please install them from a command-line with:

````
sudo apt-get install libudev-dev libdrm-dev libgconf2-dev libgcrypt11-dev libpci-dev libxtst-dev
````

[It has been reported](https://github.com/js-platform/node-webrtc/pull/75#issuecomment-35166221) that some users would need to install some more dev libraries:

````
sudo apt-get install g++ python libnss3-dev libasound2-dev libpulse-dev libjpeg62-dev libxv-dev libgtk2.0-dev libexpat1-dev
````

Maybe some of this dependencies would be removed in future revisions of the library, but to be on the safe side, it's better that you install all of them. If you need to install any other library, please open an issue so we can update the list.

Also make sure a JDK version is installed and that the JAVA_HOME environment variable is present and pointing to the correct jdk version when installing the module. JDK version 1.6.0_45 from Oracle is reported to work. Using a openjdk version may cause failure of compilation. You can check the value of the JAVA_HOME environment variable with the following command:

````
printenv JAVA_HOME
````

## Install

The easiest way to install is via npm:

````
npm install wrtc
````

If you want to work from source:

````
git clone https://github.com/js-platform/node-webrtc.git
cd node-webrtc
npm install
````

## Notes

* Development is focused on Linux and OSX at the moment. If you're on another platform, but build process may not work for you. Feel free to comment on existing issues or open new issues and add the specific problems you're having.

* ~~Only RTP data channels are supported at the moment. There's an open issue in `libjingle` and I'm working on fixing it so that I can enable SCTP data channels. See [#5](https://github.com/modeswitch/node-webrtc/issues/5) for more details.~~ SCTP data channels are supported now!

* You will need to use the latest development version of Chrome and launch it with `--enable-data-channels`.

* ~~Firefox is not yet supported due to lack of SCTP data channel support (see above).~~ Both Firefox and newer versions of Chrome are supported.

# Tests

Once everything is built, try `npm test` as a sanity check.

## bridge.js
You can run the data channel demo by `node examples/bridge.js` and browsing to `examples/peer.html` in `chrome --enable-data-channels`.

usage:
````
node examples/bridge.js [-h <host>] [-p <port>] [-ws <ws port>]
````
options:
````
-h  host IP for the webserver that will serve the static files (default 127.0.0.1)
-p  host port for the webserver that will serve the static files (default 8080)
-ws port of the Web Socket server (default 9001)
````

If the bridge and peer are on different machines, you can pass the bridge address to the peer by:
````
http://<webserver>/peer.html?<sockertserver:port>
````
By default the bridge will be the same IP as the webserver and will listen on port 9001.

The output from `bridge.js` should look like:
````
ws connected
{ type: 'offer',
  sdp: 'v=0\r\no=- 5318025751550154815 2 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\na=group:BUNDLE audio data\r\na=msid-semantic: WMS\r\nm=audio 1 RTP/SAVPF 111 103 104 0 8 106 105 13 126\r\nc=IN IP4 0.0.0.0\r\na=rtcp:1 IN IP4 0.0.0.0\r\na=ice-ufrag:swRmnysjKNa4Yo0w\r\na=ice-pwd:gOlHZ2PcJfDdAH6Qx2/VGK+B\r\na=ice-options:google-ice\r\na=fingerprint:sha-256 B6:4C:9E:EF:8C:57:49:70:D7:06:8F:E4:A9:8D:06:A6:56:E4:AA:06:26:65:35:37:44:88:94:5D:13:F8:39:EB\r\na=setup:actpass\r\na=mid:audio\r\na=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level\r\na=recvonly\r\na=rtcp-mux\r\na=crypto:1 AES_CM_128_HMAC_SHA1_80 inline:lHlR9Er2VhuoFn/Q5dJaEeoYKxYgDStgl8pL99Uw\r\na=rtpmap:111 opus/48000/2\r\na=fmtp:111 minptime=10\r\na=rtpmap:103 ISAC/16000\r\na=rtpmap:104 ISAC/32000\r\na=rtpmap:0 PCMU/8000\r\na=rtpmap:8 PCMA/8000\r\na=rtpmap:106 CN/32000\r\na=rtpmap:105 CN/16000\r\na=rtpmap:13 CN/8000\r\na=rtpmap:126 telephone-event/8000\r\na=maxptime:60\r\nm=application 1 RTP/SAVPF 101\r\nc=IN IP4 0.0.0.0\r\na=rtcp:1 IN IP4 0.0.0.0\r\na=ice-ufrag:swRmnysjKNa4Yo0w\r\na=ice-pwd:gOlHZ2PcJfDdAH6Qx2/VGK+B\r\na=ice-options:google-ice\r\na=fingerprint:sha-256 B6:4C:9E:EF:8C:57:49:70:D7:06:8F:E4:A9:8D:06:A6:56:E4:AA:06:26:65:35:37:44:88:94:5D:13:F8:39:EB\r\na=setup:actpass\r\na=mid:data\r\na=sendrecv\r\nb=AS:30\r\na=rtcp-mux\r\na=crypto:1 AES_CM_128_HMAC_SHA1_80 inline:lHlR9Er2VhuoFn/Q5dJaEeoYKxYgDStgl8pL99Uw\r\na=rtpmap:101 google-data/90000\r\na=ssrc:372397997 cname:qfTTwddQpZ1YNA9v\r\na=ssrc:372397997 msid:reliable reliable\r\na=ssrc:372397997 mslabel:reliable\r\na=ssrc:372397997 label:reliable\r\n' }
signaling state change: 3
ondatachannel reliable connecting
{ type: 'answer',
  sdp: 'v=0\r\no=- 2653351315402696646 2 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\na=group:BUNDLE audio data\r\na=msid-semantic: WMS\r\nm=audio 1 RTP/SAVPF 111 103 104 0 8 106 105 13 126\r\nc=IN IP4 0.0.0.0\r\na=rtcp:1 IN IP4 0.0.0.0\r\na=ice-ufrag:SZIEfsefmFpKF0t0\r\na=ice-pwd:E0HUC20ZF8bEMZg0XS48h2Gf\r\na=fingerprint:sha-256 A7:78:B5:08:A8:2A:51:A6:EB:7D:87:DB:82:CF:7C:77:ED:59:C7:45:84:24:9E:F3:CA:18:80:33:66:6C:DE:DC\r\na=setup:active\r\na=mid:audio\r\na=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level\r\na=sendonly\r\na=rtcp-mux\r\na=rtpmap:111 opus/48000/2\r\na=fmtp:111 minptime=10\r\na=rtpmap:103 ISAC/16000\r\na=rtpmap:104 ISAC/32000\r\na=rtpmap:0 PCMU/8000\r\na=rtpmap:8 PCMA/8000\r\na=rtpmap:106 CN/32000\r\na=rtpmap:105 CN/16000\r\na=rtpmap:13 CN/8000\r\na=rtpmap:126 telephone-event/8000\r\na=maxptime:60\r\nm=application 1 RTP/SAVPF 101\r\nc=IN IP4 0.0.0.0\r\na=rtcp:1 IN IP4 0.0.0.0\r\na=ice-ufrag:SZIEfsefmFpKF0t0\r\na=ice-pwd:E0HUC20ZF8bEMZg0XS48h2Gf\r\na=fingerprint:sha-256 A7:78:B5:08:A8:2A:51:A6:EB:7D:87:DB:82:CF:7C:77:ED:59:C7:45:84:24:9E:F3:CA:18:80:33:66:6C:DE:DC\r\na=setup:active\r\na=mid:data\r\na=sendrecv\r\nb=AS:30\r\na=rtcp-mux\r\na=rtpmap:101 google-data/90000\r\na=ssrc:1808182356 cname:ZdW7DJmRn7UTH+Bw\r\na=ssrc:1808182356 msid:reliable reliable\r\na=ssrc:1808182356 mslabel:reliable\r\na=ssrc:1808182356 label:reliable\r\n' }
signaling state change: 0
ice connection state change: 1
ice gathering state change: 1
ice gathering state change: 1
awaiting data channels
ice gathering state change: 2
ice connection state change: 2
WARNING: no real random source present!
onopen
complete
onmessage { '0': 97,
  '1': 98,
  '2': 99,
  '3': 100,
  '4': 101,
  '5': 102,
  slice: [Function: slice],
  byteLength: 6 }
````

and from `peer.html` (in the console):
````
signaling state change:  have-local-offer peer.js:80
signaling state change:  stable peer.js:80
awaiting data channels peer.js:65
ice connection state change:  checking peer.js:84
ice connection state change:  connected peer.js:84
onopen peer.js:117
complete
onmessage
MessageEvent {ports: Array[0], data: "fedcba", source: null, lastEventId: "", origin: ""…}
 peer.js:128
````

Note that the example is sending a string "abcdef" because RTP data channels don't support arraybuffers yet. This will change when we get SCTP data channels.
