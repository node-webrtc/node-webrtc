'use strict';

const test = require('tape');

const RTCPeerConnection = require('..').RTCPeerConnection;
const RTCVideoSource = require('..').RTCVideoSource;

function tick() {
  return new Promise(resolve => setTimeout(resolve));
}

function printSource(source) {
  console.log(source);
}

function printTrack(track) {
  console.log(track);
}

test('simple usage', async t => {
  await (async () => {
    const source = new RTCVideoSource();
    printSource(source);
    await tick();

    source.onFrame();
    await tick();

    const track = source.createTrack();
    printTrack(track);
    await tick();

    const clonedTrack = track.clone();
    printTrack(clonedTrack);
    await tick();

    source.onFrame();
    await tick();

    track.stop();
    printTrack(track);
    await tick();

    source.onFrame();
    await tick();

    clonedTrack.stop();
    printTrack(clonedTrack);
    await tick();

    source.onFrame();
    await tick();
  })();

  if (typeof gc === 'function') {
    gc();
  }

  t.end();
});

test('getStats()', async t => {
  const pc1 = new RTCPeerConnection();
  const pc2 = new RTCPeerConnection();

  [[pc1, pc2], [pc2, pc1]].forEach(([pcA, pcB]) => {
    pcA.onicecandidate = ({ candidate }) => {
      if (candidate) {
        pcB.addIceCandidate(candidate);
      }
    };
  });

  const source = new RTCVideoSource();
  const track = source.createTrack();
  pc1.addTrack(track);

  const offer = await pc1.createOffer();
  await pc1.setLocalDescription(offer);
  await pc2.setRemoteDescription(offer);
  const answer = await pc2.createAnswer();
  await pc2.setLocalDescription(answer);
  await pc1.setRemoteDescription(answer);

  let stats;
  do {
    source.onFrame();
    const report = await pc1.getStats();
    stats = [...report.values()]
      .find(stats => stats.trackIdentifier === track.id
                  && stats.frameWidth > 0
                  && stats.frameHeight > 0);
  } while (!stats);

  t.equal(stats.frameWidth, 1280);
  t.equal(stats.frameHeight, 720);

  track.stop();
  pc1.close();
  pc2.close();

  t.end();
});
