'use strict';

const tape = require('tape');

const { RTCPeerConnection } = require('..');

// NOTE(mroberts): These limits were tested on macOS.
const maxOrderedAndReliableSize = 262144;
// const maxUnorderedAndUnreliableSize = 262144;

async function test(t, size, options) {
  const pc1 = new RTCPeerConnection();
  const pc2 = new RTCPeerConnection();
  [[pc1, pc2], [pc2, pc1]].forEach(([pc1, pc2]) => {
    pc1.onicecandidate = ({ candidate }) => {
      if (candidate) {
        pc2.addIceCandidate(candidate);
      }
    };
  });

  const dc1 = pc1.createDataChannel('test', options);
  const dc2Promise = new Promise(resolve => {
    pc2.ondatachannel = ({ channel }) => resolve(channel);
  });

  const offer = await pc1.createOffer();
  await Promise.all([
    pc1.setLocalDescription(offer),
    pc2.setRemoteDescription(offer)
  ]);

  const answer = await pc2.createAnswer();
  await Promise.all([
    pc2.setLocalDescription(answer),
    pc1.setRemoteDescription(answer)
  ]);

  const dc2 = await dc2Promise;

  const remoteBuf1Promise = new Promise(resolve => {
    dc2.onmessage = ({ data }) => resolve(data);
  });
  const buf1 = new Uint8Array(size);
  buf1.forEach((x, i) => { buf1[i] = Math.random() * 255; });
  dc1.send(buf1);
  const remoteBuf1 = new Uint8Array(await remoteBuf1Promise);

  const remoteBuf2Promise = new Promise(resolve => {
    dc2.onmessage = ({ data }) => resolve(data);
  });
  const buf2 = new Uint8Array(size);
  buf2.forEach((x, i) => { buf2[i] = Math.random() * 255; });
  dc1.send(buf2);
  const remoteBuf2 = new Uint8Array(await remoteBuf2Promise);

  t.deepEqual(remoteBuf1.size, buf1.size);
  t.ok(remoteBuf1.every(function(x, i) {
    return x === buf1[i];
  }), 'every element of the first ArrayBuffer is zero');

  t.deepEqual(remoteBuf2.size, buf2.size);
  t.ok(remoteBuf2.every(function(x, i) {
    return x === buf2[i];
  }), 'every element of the second ArrayBuffer is one');

  t.end();

  pc1.close();
  pc2.close();
}

tape('receiving two ArrayBuffers works (ordered, reliable)', t => {
  test(t, maxOrderedAndReliableSize, {});
});

if (process.platform !== 'win32') {
  /* tape('receiving two ArrayBuffers works (unordered, unreliable)', t => {
    test(t, maxUnorderedAndUnreliableSize, {
      ordered: false,
      maxRetransmits: 0
    });
  }); */
}
