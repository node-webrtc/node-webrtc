'use strict';

const test = require('tape');

const RTCPeerConnection = require('..').RTCPeerConnection;

/* cjs 21-Aug-2019 - after replacing MediaStream objects in RtpTransceiverInit
 * with a simple array of streamID strings, this test became much harder to
 * pass.  It may make sense to remove it altogether, rather than check for a
 * non-object.
 */
test('passing something that\'s not a RtpTransceiverInit fails', t => {
  const pc = new RTCPeerConnection();
  t.throws(() => pc.addTransceiver('audio', 57), /Expected an object/);
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
