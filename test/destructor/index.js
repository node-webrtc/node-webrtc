'use strict';

const test = require('tape');

const { MediaStream, RTCPeerConnection, RTCSessionDescription } = require('../..');

const checkDestructor = require('./util');

const sdp = [
  'v=0',
  'o=- 0 1 IN IP4 0.0.0.0',
  's=-',
  't=0 0',
  'a=group:BUNDLE audio',
  'a=msid-semantic:WMS *',
  'a=ice-ufrag:0000',
  'a=ice-pwd:0000000000000000000000',
  'a=fingerprint:sha-256 00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00',
  'm=audio 9 UDP/TLS/RTP/SAVPF 109 9 0 8 101',
  'c=IN IP4 0.0.0.0',
  'a=mid:audio',
  'a=sendrecv',
  'a=setup:actpass',
  'a=rtpmap:109 opus/48000/2',
  'a=rtpmap:9 G722/8000/1',
  'a=rtpmap:0 PCMU/8000',
  'a=rtpmap:8 PCMA/8000',
  'a=rtpmap:101 PCMA/16000',
  'a=rtcp-mux',
  'a=ssrc:1 cname:0',
  'a=ssrc:1 msid:stream 123',
].join('\r\n') + '\r\n';

test('RTCPeerConnection\'s destructor fires', async t => {
  t.plan(1);
  await checkDestructor(
    'RTCPeerConnection',
    () => new RTCPeerConnection(),
    pc => pc.close());
  t.pass();
});

// NOTE(mroberts): We can test RTCRtpReceivers and remote MediaStreamTracks in
// the same way.
[
  ['RTCRtpReceiver', 'RTCRtpReceiver'],
  ['MediaStreamTrack', 'remote MediaStreamTrack']
].forEach(([type, description]) => {
  test(`${description}'s destructor fires`, async t => {
    t.plan(2);
    await checkDestructor(
      type,
      async () => {
        const pc = new RTCPeerConnection();
        const offer = new RTCSessionDescription({ type: 'offer', sdp });
        await pc.setRemoteDescription(offer);
        t.equal(pc.getReceivers().length, 1);
        return pc;
      },
      pc => pc.close());
    t.pass();
  });
});

test('RTCRtpSender\'s destructor fires', async t => {
  t.plan(2);
  await checkDestructor(
    'RTCRtpSender',
    async () => {
      const pc = new RTCPeerConnection();
      const offer = new RTCSessionDescription({ type: 'offer', sdp });
      await pc.setRemoteDescription(offer);
      pc.getReceivers().forEach(receiver => pc.addTrack(receiver.track, new MediaStream()));
      t.equal(pc.getSenders().length, pc.getReceivers().length);
      return pc;
    },
    pc => pc.close());
  t.pass();
});

test('RTCDataChannel\'s destructor fires', async t => {
  t.plan(1);
  await checkDestructor(
    'RTCDataChannel',
    () => {
      const pc = new RTCPeerConnection();
      const dc = pc.createDataChannel('test');
      // TODO(mroberts): It shouldn't be necessary to have to call `close`;
      // closing the RTCPeerConnection should be sufficient.
      dc.close();
      return pc;
    },
    pc => pc.close());
  t.pass();
});
