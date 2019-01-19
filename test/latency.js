'use strict';

const { performance } = require('perf_hooks');
const tape = require('tape');

const { RTCVideoSink, RTCVideoSource } = require('..').nonstandard;

const { fromBits, toBits } = require('./lib/bits');
const { I420Frame } = require('./lib/frame');
const { negotiateRTCPeerConnections } = require('./lib/pc');
const { printBitsI420, readBitsI420 } = require('./lib/timestamp');

function average(xs) {
  return xs.reduce((y, x) => y + x, 0) / xs.length;
}

async function measureTimeFromRTCVideoSourceToRTCVideoSink(source, sink, width, height, n) {
  n = typeof n === 'number' ? n : 300;

  const inputFrame = new I420Frame(width, height);
  const timestamps = [];

  const times = [];

  sink.onframe = outputFrame => {
    if (times.length < n) {
      const receivedAt = performance.now();

      const outputBits = readBitsI420(outputFrame);
      const i = fromBits(outputBits);
      const sentAt = timestamps[i];

      const time = receivedAt - sentAt;
      times.push(time);
    }
  };

  while (times.length < n) {
    const i = times.length;
    const inputBits = toBits(i);
    printBitsI420(inputFrame, inputBits);

    const sentAt = performance.now();
    timestamps[i] = sentAt;

    source.onFrame(inputFrame);

    await new Promise(resolve => setTimeout(resolve));
  }

  return average(times);
}

async function measureTimeFromRTCVideoSourceToLocalRTCVideoSink(width, height, n) {
  const source = new RTCVideoSource();
  const track = source.createTrack();
  const sink = new RTCVideoSink(track);
  try {
    return await measureTimeFromRTCVideoSourceToRTCVideoSink(source, sink, width, height, n);
  } catch (error) {
    throw error;
  } finally {
    sink.stop();
    track.stop();
  }
}

async function measureTimeFromRTCVideoSourceToRemoteRTCVideoSink(width, height, n) {
  const source = new RTCVideoSource();
  const localTrack = source.createTrack();
  try {
    const [pc1, pc2] = await negotiateRTCPeerConnections({
      withPc1(pc1) {
        pc1.addTrack(localTrack);
      }
    });
    try {
      const remoteTrack = pc2.getReceivers()[0].track;
      const sink = new RTCVideoSink(remoteTrack);
      try {
        return await measureTimeFromRTCVideoSourceToRTCVideoSink(source, sink, width, height, n);
      } catch (error) {
        throw error;
      } finally {
        sink.stop();
      }
    } catch (error) {
      throw error;
    } finally {
      pc1.close();
      pc2.close();
    }
  } catch (error) {
    throw error;
  } finally {
    localTrack.stop();
  }
}

function testTimeFromRTCVideoSourceToLocalVideoSink(t, width, height) {
  t.test(`Average Time from RTCVideoSource to Local RTCVideoSink (${width} x ${height})`, async t => {
    const averageTime = await measureTimeFromRTCVideoSourceToLocalRTCVideoSink(width, height);
    console.log(`#
#  ${averageTime} ms
#
`);
    t.end();
  });
}

function testTimeFromRTCVideoSourceToRemoteVideoSink(t, width, height) {
  t.test(`Average Time from RTCVideoSource to Remote RTCVideoSink (${width} x ${height})`, async t => {
    const averageTime = await measureTimeFromRTCVideoSourceToRemoteRTCVideoSink(width, height);
    console.log(`#
#  ${averageTime} ms
#
`);
    t.end();
  });
}

async function measureTimeThroughRTCDataChannel(localDataChannel, remoteDataChannel, n) {
  n = typeof n === 'number' ? n : 300;

  const times = [];

  remoteDataChannel.addEventListener('message', ({ data }) => {
    if (times.length < n) {
      const timestamp = Number.parseFloat(data);
      const time = performance.now() - timestamp;
      times.push(time);
    }
  });

  while (times.length < n) {
    const timestamp = performance.now();
    localDataChannel.send(`${timestamp}`);
    await new Promise(resolve => setTimeout(resolve));
  }

  return average(times);
}

async function measureTimeThroughUnorderedUnreliableRTCDataChannel() {
  let localDataChannel = null;
  let remoteDataChannelPromise = null;
  const [pc1, pc2] = await negotiateRTCPeerConnections({
    withPc1(pc1) {
      localDataChannel = pc1.createDataChannel('test', {
        maxRetransmits: 0,
        ordered: false
      });
    },
    withPc2(pc2) {
      remoteDataChannelPromise = new Promise(resolve => {
        pc2.addEventListener('datachannel', ({ channel }) => resolve(channel));
      });
    }
  });
  try {
    const remoteDataChannel = await remoteDataChannelPromise;
    return await measureTimeThroughRTCDataChannel(localDataChannel, remoteDataChannel);
  } catch (error) {
    throw error;
  } finally {
    pc1.close();
    pc2.close();
  }
}

function testTimeThroughUnorderedUnreliableRTCDataChannel(t) {
  t.test('Average Time through Unordered, Unreliable RTCDataChannel', async t => {
    const averageTime = await measureTimeThroughUnorderedUnreliableRTCDataChannel();
    console.log(`#
#  ${averageTime} ms
#
`);
    t.end();
  });
}

testTimeFromRTCVideoSourceToLocalVideoSink(tape,  160, 120);
testTimeFromRTCVideoSourceToLocalVideoSink(tape,  320, 240);
testTimeFromRTCVideoSourceToLocalVideoSink(tape,  640, 480);
testTimeFromRTCVideoSourceToLocalVideoSink(tape, 1280, 720);

testTimeFromRTCVideoSourceToRemoteVideoSink(tape,  160, 120);
testTimeFromRTCVideoSourceToRemoteVideoSink(tape,  320, 240);
testTimeFromRTCVideoSourceToRemoteVideoSink(tape,  640, 480);
testTimeFromRTCVideoSourceToRemoteVideoSink(tape, 1280, 720);

testTimeThroughUnorderedUnreliableRTCDataChannel(tape);
