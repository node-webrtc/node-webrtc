'use strict';

var tape = require('tape');
var wrtc = require('..');

var MediaStream = wrtc.MediaStream;
var MediaStreamTrack = wrtc.MediaStreamTrack;
var RTCPeerConnection = wrtc.RTCPeerConnection;
var RTCRtpReceiver = wrtc.RTCRtpReceiver;
var RTCSessionDescription = wrtc.RTCSessionDescription;

var sdp1 = [
  'v=0',
  'o=- 0 1 IN IP4 0.0.0.0',
  's=-',
  't=0 0',
  'a=group:BUNDLE audio video',
  'a=msid-semantic:WMS *',
  'a=ice-ufrag:0000',
  'a=ice-pwd:0000000000000000000000',
  'a=fingerprint:sha-256 00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00',
  'm=audio 9 UDP/TLS/RTP/SAVPF 109 9 0 8 101',
  'c=IN IP4 0.0.0.0',
  'a=mid:audio',
  'a=sendrecv',
  'a=rtpmap:109 opus/48000/2',
  'a=rtpmap:9 G722/8000/1',
  'a=rtpmap:0 PCMU/8000',
  'a=rtpmap:8 PCMA/8000',
  'a=rtpmap:101 PCMA/16000',
  'a=rtcp-mux',
  'a=ssrc:1 cname:0',
  'a=ssrc:1 msid:stream 123',
  'm=video 9 UDP/TLS/RTP/SAVPF 120 121 126 97',
  'c=IN IP4 0.0.0.0',
  'a=mid:video',
  'a=sendrecv',
  'a=rtpmap:120 VP8/90000',
  'a=rtpmap:121 VP9/90000',
  'a=rtpmap:126 H264/90000',
  'a=rtpmap:97 H264/180000',
  'a=rtcp-mux',
  'a=ssrc:2 cname:0',
  'a=ssrc:2 msid:stream 456'
].join('\r\n') + '\r\n';

var sdp2 = [
  'v=0',
  'o=- 0 2 IN IP4 0.0.0.0',
  's=-',
  't=0 0',
  'a=group:BUNDLE audio video',
  'a=msid-semantic:WMS *',
  'a=ice-ufrag:0000',
  'a=ice-pwd:0000000000000000000000',
  'a=fingerprint:sha-256 00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00',
  'm=audio 9 UDP/TLS/RTP/SAVPF 109 9 0 8 101',
  'c=IN IP4 0.0.0.0',
  'a=mid:audio',
  'a=recvonly',
  'a=rtpmap:109 opus/48000/2',
  'a=rtpmap:9 G722/8000/1',
  'a=rtpmap:0 PCMU/8000',
  'a=rtpmap:8 PCMA/8000',
  'a=rtpmap:101 PCMA/16000',
  'a=rtcp-mux',
  'm=video 9 UDP/TLS/RTP/SAVPF 120 121 126 97',
  'c=IN IP4 0.0.0.0',
  'a=mid:video',
  'a=recvonly',
  'a=rtpmap:120 VP8/90000',
  'a=rtpmap:121 VP9/90000',
  'a=rtpmap:126 H264/90000',
  'a=rtpmap:97 H264/180000',
  'a=rtcp-mux',
].join('\r\n') + '\r\n';

tape('applying a remote offer creates receivers (checked via .getReceivers)', function(t) {
  // NOTE(mroberts): Create and close the RTCPeerConnection inside a Promise,
  // then delay with setTimeout so that we can test accessing getReceivers after
  // the RTCPeerConnection's internals have been destroyed.
  return Promise.resolve().then(function() {
    var pc = new RTCPeerConnection();
    var offer = new RTCSessionDescription({ type: 'offer', sdp: sdp1 });
    return pc.setRemoteDescription(offer).then(function() {
      pc.close();
      return pc.getReceivers();
    });
  }).then(function(receivers) {
    return new Promise(function(resolve) { setTimeout(resolve.bind(null, receivers)); });
  }).then(function(receivers) {
    t.equal(receivers.length, 2, 'getReceivers returns an Array with length 2');
    t.ok(receivers.every(function(receiver) {
      return receiver instanceof RTCRtpReceiver;
    }), 'each entry in the Array is an RTCRtpReceiver');
    t.ok(receivers[0].track instanceof MediaStreamTrack, 'the first RTCRtpReceiver\'s .track is a MediaStreamTrack');
    t.equal(receivers[0].track.kind, 'audio', 'the first RTCRtpReceiver\'s .track has .kind "audio"');
    t.equal(receivers[0].track.id, '123', 'the first RTCRtpReceiver\'s .track has .id "123"');
    t.equal(receivers[0].track.enabled, true, 'the first RTCRtpReceiver\'s .track has .enabled true');
    t.equal(receivers[0].track.readyState, 'ended', 'the first RTCRtpReceiver\'s .track has .readyState "ended"');
    t.ok(receivers[1].track instanceof MediaStreamTrack, 'the second RTCRtpReceiver\'s .track is a MediaStreamTrack');
    t.equal(receivers[1].track.kind, 'video', 'the second RTCRtpReceiver\'s .track has .kind "video"');
    t.equal(receivers[1].track.id, '456', 'the second RTCRtpReceiver\'s .track has .id "456"');
    t.equal(receivers[1].track.enabled, true, 'the second RTCRtpReceiver\'s .track has .enabled true');
    t.equal(receivers[1].track.readyState, 'ended', 'the second RTCRtpReceiver\'s .track has .readyState "ended"');
    t.end();
  });
});

tape('applying a remote offer creates receivers (checked via .ontrack)', function(t) {
  // NOTE(mroberts): Create and close the RTCPeerConnection inside a Promise,
  // then delay with setTimeout so that we can test accessing getReceivers after
  // the RTCPeerConnection's internals have been destroyed.
  return Promise.resolve().then(function() {
    var pc = new RTCPeerConnection();
    var offer = new RTCSessionDescription({ type: 'offer', sdp: sdp1 });
    var trackEventPromise1 = new Promise(function(resolve) {
      pc.ontrack = resolve;
    });
    var trackEventPromise2 = trackEventPromise1.then(function() {
      return new Promise(function(resolve) {
        pc.ontrack = resolve;
      });
    });
    return pc.setRemoteDescription(offer).then(function() {
      pc.close();
      return Promise.all([trackEventPromise1, trackEventPromise2]);
    });
  }).then(function(trackEvents) {
    return new Promise(function(resolve) { setTimeout(resolve.bind(null, trackEvents)); });
  }).then(function(trackEvents) {
    t.ok(trackEvents.every(function(trackEvent) {
      return trackEvent.receiver instanceof RTCRtpReceiver;
    }), 'each RTCTrackEvent\'s .receiver is an RTCRtpReceiver');
    t.ok(trackEvents.every(function(trackEvent) {
      return trackEvent.track instanceof MediaStreamTrack;
    }), 'each RTCTrackEvent\'s .track is a MediaStreamTrack');
    t.ok(trackEvents.every(function(trackEvent) {
      return trackEvent.track === trackEvent.receiver.track;
    }), 'each RTCTrackEvent\'s .track equals its .receiver.track');
    t.ok(trackEvents.every(function(trackEvent) {
      return Array.isArray(trackEvent.streams)
        && trackEvent.streams.length === 1
        && trackEvent.streams[0] instanceof MediaStream;
    }), 'each RTCTrackEvent\'s .streams is an Array containing a single MediaStreamTrack');
    t.ok(trackEvents.every(function(trackEvent) {
      return trackEvent.streams[0].id === 'stream';
    }), 'each RTCTrackEvent\'s MediaStream has .id "stream"');
    t.ok(trackEvents.every(function(trackEvent) {
      return trackEvent.streams[0].getTracks().indexOf(trackEvent.track) > -1;
    }), 'each RTCTrackEvent\'s MediaStream contains its MediaStreamTrack (checked via kind-generic method)');
    t.ok(trackEvents.every(function(trackEvent) {
      return trackEvent.streams[0][trackEvent.track.kind === 'audio'
        ? 'getAudioTracks' : 'getVideoTracks'
      ]().indexOf(trackEvent.track) > -1;
    }), 'each RTCTrackEvent\'s MediaStream contains its MediaStreamTrack (checked via kind-specific method)');
    t.ok(trackEvents.every(function(trackEvent) {
      return trackEvent.streams[0].active === false;
    }), 'each RTCTrackEvent\'s MediaStream\'s .active is false');
    t.ok(trackEvents.reduce(function(trackEvent1, trackEvent2) {
      return trackEvent1.streams[0] === trackEvent2.streams[0];
    }), 'the RTCTrackEvent\'s MediaStreams are the same');
    t.end();
  });
});

tape('applying a remote offer and then applying a local answer causes .getParameters to return values', function(t) {
  var pc = new RTCPeerConnection();
  var offer = new RTCSessionDescription({ type: 'offer', sdp: sdp1 });
  return pc.setRemoteDescription(offer).then(function() {
    return pc.createAnswer();
  }).then(function(answer) {
    return pc.setLocalDescription(answer);
  }).then(function() {
    return pc.getReceivers();
  }).then(function(receivers) {
    t.equal(receivers[0].track.readyState, 'live', 'the audio RTCRtpReceiver\'s .track has .readyState "live"');
    t.equal(receivers[1].track.readyState, 'live', 'the video RTCRtpReceiver\'s .track has .readyState "live"');
    t.deepEqual(receivers[0].getParameters(), {
      headerExtensions: [],
      codecs: [
        {
          payloadType: 109,
          mimeType: 'audio/opus',
          clockRate: 48000,
          channels: 2,
          sdpFmtpLine: 'a=fmtp:109 useinbandfec=1; minptime=10'
        },
        {
          payloadType: 9,
          mimeType: 'audio/G722',
          clockRate: 8000,
          channels: 1
        },
        {
          payloadType: 0,
          mimeType: 'audio/PCMU',
          clockRate: 8000,
          channels: 1
        },
        {
          payloadType: 8,
          mimeType: 'audio/PCMA',
          clockRate: 8000,
          channels: 1
        }
      ],
      encodings: []
    }, 'the audio RTCRtpReceiver\'s .getParameters() returns the expected RTCRtpParameters');
    t.deepEqual(receivers[1].getParameters(), {
      headerExtensions: [],
      codecs: [
        {
          payloadType: 120,
          mimeType: 'video/VP8',
          clockRate: 90000
        },
        {
          payloadType: 121,
          mimeType: 'video/VP9',
          clockRate: 90000
        }
      ],
      encodings: []
    }, 'the video RTCRtpReceiver\'s .getParameters() returns the expected RTCRtpParameters');
    pc.close();
    t.end();
  });
});

tape('negotiating MediaStreamTracks and then renegotiating without them', function(t) {
  var pc = new RTCPeerConnection();
  var offer1 = new RTCSessionDescription({ type: 'offer', sdp: sdp1 });
  return pc.setRemoteDescription(offer1).then(function() {
    return pc.createAnswer();
  }).then(function(answer1) {
    return pc.setLocalDescription(answer1);
  }).then(function() {
    var offer2 = new RTCSessionDescription({ type: 'offer', sdp: sdp2 });
    return pc.setRemoteDescription(offer2);
  }).then(function() {
    return pc.createAnswer();
  }).then(function(answer2) {
    return pc.setLocalDescription(answer2);
  }).then(function() {
    var receivers = pc.getReceivers();
    t.equal(receivers[0].track.readyState, 'ended', 'the audio RTCRtpReceiver\'s .track has .readyState "live"');
    t.equal(receivers[1].track.readyState, 'ended', 'the video RTCRtpReceiver\'s .track has .readyState "live"');
    pc.close();
    t.end();
  });
});

tape('accessing remote MediaStreamTrack after RTCPeerConnection is destroyed', function(t) {
  return Promise.resolve().then(function() {
    var pc = new RTCPeerConnection();
    var offer1 = new RTCSessionDescription({ type: 'offer', sdp: sdp1 });
    return pc.setRemoteDescription(offer1).then(function() {
      return pc.createAnswer();
    }).then(function(answer) {
      return pc.setLocalDescription(answer);
    }).then(function() {
      pc.close();
      return new Promise(function(resolve) { setTimeout(resolve); });
    }).then(function() {
      return pc.getReceivers().map(function(receiver) {
        return receiver.track;
      });
    });
  }).then(function(mediaStreamTracks) {
    t.ok(mediaStreamTracks.every(function(mediaStreamTrack) {
      return mediaStreamTrack.readyState === 'ended';
    }), 'every MediaStreamTrack is ended');
    t.end();
  });
});
