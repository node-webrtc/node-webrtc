'use strict';

const test = require('tape');

const { getUserMedia } = require('..');
const { RTCAudioSink } = require('..').nonstandard;

test('RTCAudioSink', t => {
  return getUserMedia({ audio: true }).then(stream => {
    const track = stream.getAudioTracks()[0];
    const sink = new RTCAudioSink(track);
    t.ok(!sink.stopped, 'RTCAudioSink initially is not stopped');
    sink.stop();
    t.ok(sink.stopped, 'RTCAudioSink is finally stopped');
    track.stop();
    t.end();
  });
});
