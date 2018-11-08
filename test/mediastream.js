'use strict';

var tape = require('tape');
var wrtc = require('..');

var getUserMedia = wrtc.getUserMedia;
var MediaStream = wrtc.MediaStream;
var RTCPeerConnection = wrtc.RTCPeerConnection;
var RTCSessionDescription = wrtc.RTCSessionDescription;

var sdp = [
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

tape('new MediaStream()', function(t) {
  var stream = new MediaStream();
  t.equal(stream.getTracks().length, 0, 'the MediaStream does not contain any MediaStreamTracks');
  t.end();
});

tape('new MediaStream(stream)', function(t) {
  return getMediaStream().then(function(stream1) {
    var stream2 = new MediaStream(stream1);
    t.notEqual(stream2.id, stream1.id, 'the MediaStream .ids do not match');
    t.ok(stream2.getTracks().every(function(track, i) {
      return track === stream1.getTracks()[i];
    }) && stream1.getTracks().every(function(track, i) {
      return track === stream2.getTracks()[i];
    }), 'the MediaStreams\' MediaStreamTracks are the same');
    t.end();
  });
});

tape('new MediaStream(tracks)', function(t) {
  return getMediaStream().then(function(stream1) {
    var tracks = stream1.getTracks();
    var stream2 = new MediaStream(tracks);
    t.ok(stream2.getTracks().every(function(track, i) {
      return track === tracks[i];
    }) && tracks.every(function(track, i) {
      return track === stream2.getTracks()[i];
    }), 'the MediaStream\'s MediaStreamTracks match tracks');
    t.end();
  });
});

tape('.clone', function(t) {
  return getMediaStream().then(function(stream1) {
    var stream2 = stream1.clone();
    var stream3 = stream2.clone();
    // NOTE(mroberts): Weirdly, cloned video MediaStreamTracks have .readyState
    // "live"; we'll .stop them, at least until that bug is fixed.
    // stream2.getVideoTracks().forEach(function(track) {
    stream2.getTracks().forEach(function(track) {
      track.stop();
    });
    // stream3.getVideoTracks().forEach(function(track) {
    stream3.getTracks().forEach(function(track) {
      track.stop();
    });
    t.ok(
      stream1.id !== stream2.id &&
      stream2.id !== stream3.id &&
      stream3.id !== stream1.id,
      'the cloned MediaStreams have different IDs');
    t.ok(
      stream1.getTracks().length === stream2.getTracks().length &&
      stream1.getTracks().length === stream3.getTracks().length,
      'the cloned MediaStreams contain the same number of MediaStreamTracks');
    t.ok(stream1.getTracks().every(function(track, i) {
      return track.kind === stream2.getTracks()[i].kind &&
        track.kind === stream3.getTracks()[i].kind;
    }), 'the cloned MediaStreams contain the same kinds of MediaStreamTracks');
    t.ok(stream1.getTracks().every(function(track, i) {
      return track.id !== stream2.getTracks()[i].id &&
        track.id !== stream3.getTracks()[i].id;
    }), 'the cloned MediaStreams\'s MediaStreamTracks do not have the same .ids');
    t.end();
  });
});

tape.skip('.clone and .stop', function(t) {
  getUserMedia({ audio: true }).then(function(stream1) {
    var track1 = stream1.getTracks()[0];

    t.ok(stream1.active, 'stream1 is active');
    t.equal(track1.readyState, 'live', 'track1 is live');

    var stream2 = stream1.clone();
    var track2 = stream2.getTracks()[0];

    t.ok(stream2.active, 'stream2 is active');
    t.equal(track2.readyState, 'live', 'track2 is live');
    t.notEqual(stream1, stream2, 'stream1 and stream2 are different');
    t.notEqual(track1, track2, 'track1 and track2 are different');

    track1.stop();

    t.ok(!stream1.active, 'stream1 is inactive');
    t.ok(stream2.active, 'stream2 is active');
    t.equal(track1.readyState, 'ended', 'track1 is ended');
    t.equal(track2.readyState, 'live', 'track2 is live');

    track2.stop();

    t.ok(!stream2.active, 'stream2 is inactive');
    t.equal(track2.readyState, 'ended', 'track2 is ended');

    t.end();
  });
});

tape('.removeTrack and .addTrack on remote MediaStream', function(t) {
  return getMediaStream().then(function(stream) {
    var tracks = stream.getTracks();
    tracks.forEach(function(track) {
      stream.removeTrack(track);
    });
    t.equal(stream.getTracks().length, 0, 'all MediaStreamTracks are removed');
    tracks.forEach(function(track) {
      stream.addTrack(track);
    });
    t.equal(stream.getTracks().length, tracks.length, 'all MediaStreamTracks are added back');
    t.ok(stream.getTracks().every(function(track, i) {
      return track === tracks[i];
    }), 'all MediaStreamTracks added back are the same (and in the same order)');
    t.end();
  });
});

function getMediaStream() {
  var pc = new RTCPeerConnection();
  var offer = new RTCSessionDescription({ type: 'offer', sdp: sdp });
  var trackEventPromise = new Promise(function(resolve) {
    pc.ontrack = resolve;
  });
  return pc.setRemoteDescription(offer).then(function() {
    return trackEventPromise;
  }).then(function(trackEvent) {
    pc.close();
    return trackEvent.streams[0];
  });
}
