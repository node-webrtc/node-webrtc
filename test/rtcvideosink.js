'use strict';

const test = require('tape');

const { RTCVideoSink, RTCVideoSource } = require('..').nonstandard;
const { I420Frame } = require('./lib/frame');

test('RTCVideoSink', t => {
  const source = new RTCVideoSource();
  const track = source.createTrack();
  const sink = new RTCVideoSink(track);
  t.ok(!sink.stopped, 'RTCVideoSink initially is not stopped');
  const inputFrame = new I420Frame(160, 120);
  const outputFramePromise = new Promise(resolve => { sink.onframe = ({ frame }) => resolve(frame); });
  source.onFrame(inputFrame);
  return outputFramePromise.then(outputFrame => {
    t.equal(inputFrame.width, outputFrame.width);
    t.equal(inputFrame.height, outputFrame.height);
    t.deepEqual(inputFrame.data, outputFrame.data);
    sink.stop();
    t.ok(sink.stopped, 'RTCVideoSink initially is finally stopped');
    track.stop();
    t.end();
  });
});
