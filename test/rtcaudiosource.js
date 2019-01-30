/* eslint no-undefined:0 */
'use strict';

const test = require('tape');

const { RTCAudioSink, RTCAudioSource } = require('..').nonstandard;

const bitsPerSample = 8;
const sampleRate = 8000;
const numberOfFrames = 1;
const numberOfChannels = 1;

const byteLength = numberOfChannels * numberOfFrames * bitsPerSample / 8;

const audioData = new Uint8ClampedArray(byteLength);

const data = {
  audioData,
  sampleRate,
  bitsPerSample,
  numberOfChannels,
  numberOfFrames
};

test('RTCAudioSource', t => {
  const source = new RTCAudioSource();
  const track = source.createTrack();
  t.ok(!track.stopped, 'createTrack() returns a non-stopped MediaStreamTrack');

  const sink = new RTCAudioSink(track);
  const receivedDataPromise = new Promise(resolve => { sink.ondata = resolve; });

  t.equal(source.onData(data), undefined, 'onData() returns undefined');

  receivedDataPromise.then(receivedData => {
    t.deepEqual(receivedData, Object.assign({ type: 'data' }, data));

    track.stop();

    sink.stop();

    t.end();
  });
});
