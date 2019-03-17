/* eslint no-undefined:0 */
'use strict';

const test = require('tape');

const { RTCAudioSink, RTCAudioSource } = require('..').nonstandard;

function createData(bitsPerSample) {
  const sampleRate = 8000;
  const channelCount = 1;
  const numberOfFrames = sampleRate / 100;  // 10 ms

  const length = channelCount * numberOfFrames;
  const byteLength = length * bitsPerSample / 8;

  const samples = {
    8: new Int8Array(length),
    16: new Int16Array(length),
    32: new Int32Array(length)
  }[bitsPerSample] || new Uint8Array(byteLength);

  samples[0] = -1 * Math.pow(2, bitsPerSample) / 2;

  return {
    samples,
    sampleRate,
    bitsPerSample,
    channelCount,
    numberOfFrames
  };
}

function createTest(bitsPerSample) {
  test(`RTCAudioSource (${bitsPerSample}-bit)`, t => {
    const source = new RTCAudioSource();
    const track = source.createTrack();
    t.ok(!track.stopped, 'createTrack() returns a non-stopped MediaStreamTrack');

    const sink = new RTCAudioSink(track);
    const receivedDataPromise = new Promise(resolve => { sink.ondata = resolve; });

    const data = createData(bitsPerSample);
    t.equal(source.onData(data), undefined, 'onData() returns undefined');

    receivedDataPromise.then(receivedData => {
      t.deepEqual(receivedData, Object.assign({ type: 'data' }, data));

      track.stop();

      sink.stop();

      t.end();
    });
  });
}

// createTest(8);
createTest(16);
// createTest(32);
// createTest(64);
