libwebrtc stuff
----------

To build libjingle (from top-level directory):

1. Grab the depot tools (they may already be checked in, so you could skip this)
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git tools/depot_tools
export PATH=`pwd`/tools/depot_tools:$PATH

2. Get/update/build libwebrtc
mkdir -p lib/libwebrtc
cd lib/libwebrtc
gclient config http://webrtc.googlecode.com/svn/trunk
gclient sync --revision r4953
gclient runhooks
ninja -C trunk/out/Release peerconnection_client


tests
----------

Once everything is built, try `node tests/test.js` as a sanity check. You can run the data channel demo by `node tests/bridge.js` and browsing to `tests/peer.html` in `chrome --enable-data-channels`.

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
````
