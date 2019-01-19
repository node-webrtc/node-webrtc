'use strict';

const test = require('tape');

const { RTCVideoSink, RTCVideoSource } = require('..');
const { I420Frame } = require('./lib/frame');

test('RTCVideoSink', t => {
  const source = new RTCVideoSource();
  const track = source.createTrack();
  const sink = new RTCVideoSink(track);
  const inputFrame = new I420Frame(160, 120);
  const outputFramePromise = new Promise(resolve => { sink.onframe = resolve; });
  source.onFrame(inputFrame);
  return outputFramePromise.then(outputFrame => {
    t.equal(inputFrame.width, outputFrame.width);
    t.equal(inputFrame.height, outputFrame.height);
    t.deepEqual(inputFrame.data, outputFrame.data);
    sink.stop();
    track.stop();
    t.end();
  });
});
