'use strict';

const tape = require('tape');
const { RTCPeerConnection } = require('..');

tape('Calling .send(message) when .readyState is "closed" throws InvalidStateError', t => {
  const pc = new RTCPeerConnection();
  const dc = pc.createDataChannel('hello');
  pc.close();
  t.throws(() => dc.send('world'), /RTCDataChannel.readyState is not 'open'/);
  t.end();
});

tape('.maxPacketLifeTime', t => {
  const pc = new RTCPeerConnection();
  const dc1 = pc.createDataChannel('dc1');
  const dc2 = pc.createDataChannel('dc2', { maxPacketLifeTime: 0 });
  t.equal(dc1.maxPacketLifeTime, 65535);
  t.equal(dc2.maxPacketLifeTime, 0);
  pc.close();
  t.end();
});

tape('.negotiated', t => {
  const pc = new RTCPeerConnection();
  const dc1 = pc.createDataChannel('dc1');
  const dc2 = pc.createDataChannel('dc2', { negotiated: true });
  t.equal(dc1.negotiated, false);
  t.equal(dc2.negotiated, true);
  pc.close();
  t.end();
});
