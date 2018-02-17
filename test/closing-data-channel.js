'use strict';

const test = require('tape');
const { RTCPeerConnection } = require('..');

test('make sure closing an RTCDataChannel after an RTCPeerConnection has been garbage collected doesn\'t segfault', t => {
  const dc = (() => {
    const pc = new RTCPeerConnection();
    const dc = pc.createDataChannel();
    pc.close();
    return dc;
  })();

  dc.close();
  t.end();
});
