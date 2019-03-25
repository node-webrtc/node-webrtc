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

function waitForStateChange(transport, state) {
  return new Promise(resolve => {
    transport.onstatechange = () => {
      if (transport.state === state) {
        resolve();
      }
    };
  });
}

function gatherCandidates(pc) {
  const candidates = [];
  return new Promise(resolve => {
    if (pc.iceGatheringState === 'complete') {
      resolve(candidates);
    }
    pc.addEventListener('icecandidate', ({ candidate }) => {
      if (!candidate) {
        resolve(candidates);
        return;
      }
      candidates.push(candidate);
    });
  });
}

function testDtlsTransport(t, createSenderOrReceiver) {
  const [pc1, pc2] = createRTCPeerConnections();
  const senderOrReceiver = createSenderOrReceiver(pc1);
  t.equal(senderOrReceiver.transport, null, 'transport is initially null');

  const candidates1Promise = gatherCandidates(pc1);
  const candidates2Promise = gatherCandidates(pc2);

  return negotiate(pc1, pc2).then(() => {
    const { transport } = senderOrReceiver;
    t.ok(transport instanceof RTCDtlsTransport, 'transport is no longer null');
    t.equal(transport.state, 'new', '.state is initially "new"');

    const connectedPromise = waitForStateChange(transport, 'connected');

    return Promise.all([candidates1Promise, candidates2Promise]).then(candidates => {
      return Promise.all(
        candidates[0].map(candidate => pc2.addIceCandidate(candidate)).concat(
        candidates[1].map(candidate => pc1.addIceCandidate(candidate)))
      );
    }).then(() => {
      return connectedPromise;
    }).then(() => {
      t.pass('"statechange" fires in state "connected"');
      t.equal(transport.state, 'connected', '.state is still "connected"');

      pc1.close();
      pc2.close();

      t.equal(senderOrReceiver.transport, transport, 'transport is still not null');
      t.equal(transport.state, 'closed', '.state is finally "closed"');

      t.end();
    });
  });
}

test('RTCDtlsTransport', t => {
  test('accessed via RTCRtpSender .transport', t =>
    testDtlsTransport(t, pc => pc.addTransceiver('audio').sender));

  test('accessed via RTCRtpReceiver .transport', t =>
    testDtlsTransport(t, pc => pc.addTransceiver('audio').receiver));

  t.end();
});
