'use strict';

const tape = require('tape');
const { RTCAudioSink } = require('../../../../..').nonstandard;

const PitchDetector = require('../../../../lib/common/pitchdetector');
const RTCAudioSourceSineWave = require('../../../../lib/server/webrtc/rtcaudiosourcesinewave');

tape('RTCAudioSinkFrequencyDetector', t => {
  t.test('it works', t => {
    const source = new RTCAudioSourceSineWave();
    const track = source.createTrack();
    const sink = new RTCAudioSink(track);
    const pitchDetector = new PitchDetector(track);
    const e = 1;
    sink.ondata = data => {
      // FIXME(mroberts):
      data.audioData = new Int16Array(data.audioData.buffer);
      const frequency = pitchDetector.onData(data);
      if (source.frequency - e <= frequency && frequency <= source.frequency + e) {
        sink.stop();
        track.stop();
        source.close();
        t.end();
      }
    };
  });
  t.end();
});
