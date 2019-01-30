/* eslint no-undefined:0 */
'use strict';

const test = require('tape');

const { RTCAudioSource } = require('..').nonstandard;

test('RTCAudioSource', t => {
  const source = new RTCAudioSource();
  const track = source.createTrack();
  t.ok(!track.stopped, 'createTrack() returns a non-stopped MediaStreamTrack');
  track.stop();
  t.equal(source.onData(), undefined, 'onData() returns undefined');
  t.end();
});
