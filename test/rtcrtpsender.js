'use strict';

var tape = require('tape');
var wrtc = require('..');

var MediaStreamTrack = wrtc.MediaStreamTrack;
var RTCPeerConnection = wrtc.RTCPeerConnection;
var RTCRtpSender = wrtc.RTCRtpSender;
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

tape('.addTrack(track, stream)', function(t) {
  return getMediaStream().then(function(stream) {
    var pc = new RTCPeerConnection();
    t.equal(pc.getSenders().length, 0, 'initially, .getSenders() returns an empty Array');
    var tracks = stream.getTracks();
    var senders = tracks.map(function(track) {
      return pc.addTrack(track, stream);
    });
    t.equal(pc.getSenders().length, senders.length, 'then, after calling .addTrack(track, stream), .getSenders() returns a non-empty Array');
    t.ok(pc.getSenders().every(function(sender) {
      return sender instanceof RTCRtpSender;
    }), 'every element of the Array returned by .getSenders() is an RTCRtpSender');
    t.ok(pc.getSenders().every(function(sender, i) {
      return sender === senders[i];
    }), 'every RTCRtpSender returned by .addTrack(track, stream) is present in .getSenders()');
    t.ok(senders.every(function(sender, i) {
      return sender.track === tracks[i];
    }), 'every RTCRtpSender\'s .track is one of the MediaStreamTracks added');
    senders.forEach(function(sender) {
      pc.removeTrack(sender);
    });
    t.equal(pc.getSenders().length, 0, 'finally, after calling .removeTrack(sender), .getSenders() returns an empty Array again');
    t.ok(senders.every(function(sender) {
      return sender.track instanceof MediaStreamTrack;
    }), 'but every RTCRtpSender\'s .track is still non-null');
    pc.close();
    t.end();
  });
});

tape('.addTrack(track, stream) called twice', t => {
  return getMediaStream().then(stream => {
    const pc = new RTCPeerConnection();
    const [track] = stream.getTracks();
    pc.addTrack(track, stream);
    t.throws(
      () => pc.addTrack(track, stream),
      /Sender already exists for track/,
      'calling .addTrack(track, stream) with the same track twice throws'
    );
    pc.close();
    t.end();
  });
});

tape('.replaceTrack(null)', function(t) {
  return getMediaStream().then(function(stream) {
    var pc = new RTCPeerConnection();
    var senders = stream.getTracks().map(function(track) {
      return pc.addTrack(track, stream);
    });
    return Promise.all(senders.map(function(sender) {
      return sender.replaceTrack(null);
    })).then(function() {
      t.ok(senders.every(function(sender) {
        return sender.track === null;
      }), 'every RTCRtpSender\'s .track is null');
      pc.close();
      t.end();
    });
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
