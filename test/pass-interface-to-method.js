'use strict';

const test = require('tape');

const RTCPeerConnection = require('..').RTCPeerConnection;

test('passing something that\'s not a MediaStream fails', t => {
  const pc = new RTCPeerConnection();
  t.throws(() => pc.addTransceiver('audio', { streams: [{}] }), / This is not an instance of MediaStream/);
  pc.close();
  t.end();
});

test('passing something that\'s not a MediaStreamTrack fails', t => {
  const pc = new RTCPeerConnection();
  t.throws(() => pc.addTrack({}), /This is not an instance of MediaStreamTrack/);
  pc.close();
  t.end();
});

test('passing something that\'s not an RTCRtpSender fails', t => {
  const pc = new RTCPeerConnection();
  t.throws(() => pc.removeTrack({}), /This is not an instance of RTCRtpSender/);
  pc.close();
  t.end();
});
