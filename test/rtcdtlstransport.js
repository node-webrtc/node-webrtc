'use strict';

const test = require('tape');
const { Certificate } = require('@fidm/x509');

const {
  RTCDtlsTransport,
  RTCIceTransport
} = require('..');

const {
  createRTCPeerConnections,
  gatherCandidates,
  negotiate,
  waitForStateChange
} = require('./lib/pc');

async function testDtlsTransport(t, createSenderOrReceiver) {
  const [pc1, pc2] = createRTCPeerConnections({}, {}, { handleIce: false });
  const senderOrReceiver = createSenderOrReceiver(pc1);
  t.equal(senderOrReceiver.transport, null, 'transport is initially null');

  const candidates1Promise = gatherCandidates(pc1);
  const candidates2Promise = gatherCandidates(pc2);

  await negotiate(pc1, pc2);

  const { transport } = senderOrReceiver;
  t.ok(transport instanceof RTCDtlsTransport, 'transport is no longer null');
  t.equal(transport.state, 'new', '.state is initially "new"');
  t.ok(transport.iceTransport instanceof RTCIceTransport, '.iceTransport is not null');
  t.equal(transport.iceTransport.state, 'new', '.iceTransport.state is also "new"');
  t.equal(transport.iceTransport.component, 'rtp', '.iceTransport.component is "rtp"');
  t.equal(transport.iceTransport.role, 'controlling', '.iceTransport.role is "controlling"');

  const connectingPromise = waitForStateChange(transport, 'connecting');
  const connectedPromise = waitForStateChange(transport, 'connected');

  const candidates = await Promise.all([candidates1Promise, candidates2Promise]);

  await Promise.all(
    candidates[0].map(candidate => pc2.addIceCandidate(candidate)).concat(
    candidates[1].map(candidate => pc1.addIceCandidate(candidate)))
  );

  if (transport.state !== 'connected') {
    await connectingPromise;
  }
  t.ok(transport.state === 'connecting' || transport.state === 'connected', '.state transitions to "connecting" (or "connected")');
  t.ok(transport.iceTransport.state === 'checking' || transport.iceTransport.state === 'connected', '.state transitions to "checking" (or "connected")');

  await connectedPromise;

  t.pass('"statechange" fires in state "connected"');
  t.equal(transport.state, 'connected', '.state is "connected"');
  t.equal(transport.iceTransport.state, 'connected', '.iceTransport.state is also "connected"');

  const remoteCertificates = transport.getRemoteCertificates();
  t.ok(remoteCertificates.length > 0, 'getRemoteCertificates() returns at least one remote certificate');
  remoteCertificates.forEach((derBuffer, i) => {
    // NOTE(mroberts): https://stackoverflow.com/a/48309802
    const prefix = '-----BEGIN CERTIFICATE-----\n';
    const postfix = '-----END CERTIFICATE-----';
    const pemText = prefix + Buffer.from(derBuffer).toString('base64').match(/.{0,64}/g).join('\n') + postfix;
    const cert = Certificate.fromPEM(pemText);
    t.equal(cert.issuer.commonName, 'WebRTC', 'Issuer is "WebRTC"');
    t.pass(`parsed remote certificate ${i + 1}`);
  });

  pc1.close();
  pc2.close();

  t.equal(senderOrReceiver.transport, transport, 'transport is still not null');
  t.equal(transport.state, 'closed', '.state is finally "closed"');
  t.ok(transport.iceTransport instanceof RTCIceTransport, '.iceTransport is still not null');
  t.equal(transport.iceTransport.state, 'closed', '.iceTransport.state is "closed"');
  t.equal(transport.iceTransport.component, 'rtp', '.iceTransport.component is still "rtp"');
  t.equal(transport.iceTransport.role, 'controlling', '.iceTransport.role is still "controlling"');

  t.end();
}

test('RTCDtlsTransport', t => {
  test('accessed via RTCRtpSender .transport', t =>
    testDtlsTransport(t, pc => pc.addTransceiver('audio').sender));

  test('accessed via RTCRtpReceiver .transport', t =>
    testDtlsTransport(t, pc => pc.addTransceiver('audio').receiver));

  t.end();
});
