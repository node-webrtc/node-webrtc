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
