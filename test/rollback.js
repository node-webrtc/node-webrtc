'use strict';

const test = require('tape');

const { RTCPeerConnection } = require('..');

test('local rollback (wrong state)', async t => {
  const pc = new RTCPeerConnection({ sdpSemantics: 'unified-plan' });
  try {
    await pc.setLocalDescription({ type: 'rollback' });
    t.fail();
  } catch (error) {
    t.pass(error);
  } finally {
    pc.close();
    t.end();
  }
});

test('remote rollback (wrong state)', async t => {
  const pc = new RTCPeerConnection({ sdpSemantics: 'unified-plan' });
  try {
    await pc.setRemoteDescription({ type: 'rollback' });
    t.fail();
  } catch (error) {
    t.pass(error);
  } finally {
    pc.close();
    t.end();
  }
});

test('local rollback', async t => {
  const pc = new RTCPeerConnection({ sdpSemantics: 'unified-plan' });
  try {
    pc.addTransceiver('audio');
    const localDescription = await pc.createOffer();
    await pc.setLocalDescription(localDescription);
    t.equal(pc.signalingState, 'have-local-offer');
    await pc.setLocalDescription({ type: 'rollback' });
    t.equal(pc.signalingState, 'stable');
  } catch (error) {
    t.fail(error);
  } finally {
    pc.close();
    t.end();
  }
});

test('remote rollback', async t => {
  const pc1 = new RTCPeerConnection({ sdpSemantics: 'unified-plan' });
  const pc2 = new RTCPeerConnection({ sdpSemantics: 'unified-plan' });
  try {
    pc1.addTransceiver('audio');
    const remoteDescription = await pc1.createOffer();
    await pc2.setRemoteDescription(remoteDescription);
    t.equal(pc2.signalingState, 'have-remote-offer');
    await pc2.setRemoteDescription({ type: 'rollback' });
    t.equal(pc2.signalingState, 'stable');
  } catch (error) {
    t.fail();
  } finally {
    pc1.close();
    pc2.close();
    t.end();
  }
});
test('remote rollback', async t => {
  const pc1 = new RTCPeerConnection({ sdpSemantics: 'unified-plan' });
  const pc2 = new RTCPeerConnection({ sdpSemantics: 'unified-plan' });
  try {
    pc1.addTransceiver('audio');
    const remoteDescription = await pc1.createOffer();
    await pc2.setRemoteDescription(remoteDescription);
    t.equal(pc2.signalingState, 'have-remote-offer');
    await pc2.setRemoteDescription({ type: 'rollback' });
    t.equal(pc2.signalingState, 'stable');
  } catch (error) {
    t.fail();
  } finally {
    pc1.close();
    pc2.close();
    t.end();
  }
});
