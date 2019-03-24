'use strict';

const test = require('tape');

const {
  RTCDtlsTransport,
  RTCPeerConnection
} = require('..');

function createRTCPeerConnections() {
  const configuration = { sdpSemantics: 'unified-plan' };
  const pc1 = new RTCPeerConnection(configuration);
  const pc2 = new RTCPeerConnection(configuration);
  return [pc1, pc2];
}

function negotiate(offerer, answerer) {
  return offerer.createOffer().then(offer => {
    return Promise.all([
      offerer.setLocalDescription(offer),
      answerer.setRemoteDescription(offer)
    ]);
  }).then(() => {
    return answerer.createAnswer();
  }).then(answer => {
    return Promise.all([
      answerer.setLocalDescription(answer),
      offerer.setRemoteDescription(answer)
    ]);
  }).then(() => {});
}

function testDtlsTransport(t, createSenderOrReceiver) {
  const [pc1, pc2] = createRTCPeerConnections();
  const senderOrReceiver = createSenderOrReceiver(pc1);
  t.equal(senderOrReceiver.transport, null, 'initially null');
  return negotiate(pc1, pc2).then(() => {
    const { transport } = senderOrReceiver;
    t.ok(transport instanceof RTCDtlsTransport);
    t.equal(transport.state, 'new', '.state is initially "new"');
    pc1.close();
    pc2.close();
    t.equal(transport.state, 'closed', '.state is finally "closed"');
    t.end();
  });
}

test('RTCDtlsTransport', t => {
  test('accessed via RTCRtpSender .transport', t => {
    testDtlsTransport(t, pc => pc.addTransceiver('audio').sender);
  });

  test('accessed via RTCRtpReceiver .transport', t => {
    testDtlsTransport(t, pc => pc.addTransceiver('audio').receiver);
  });

  t.end();
});
