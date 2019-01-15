'use strict';

const { createCanvas, createImageData } = require('canvas');
// const { performance } = require('perf_hooks');
const tape = require('tape');

const { RTCVideoSink, RTCVideoSource, i420ToArgb32 } = require('..');

const { printTimestamp, readTimestamp } = require('./lib/timestamp');
const { I420Frame } = require('./lib/frame');

tape('RTCVideoSink latency', async t => {
  const source = new RTCVideoSource();
  const track = source.createTrack();
  const sink = new RTCVideoSink(track);

  const inputWidth = 640;
  const inputHeight = 480;
  const inputCanvas = createCanvas(inputWidth, inputHeight);
  const inputContext = inputCanvas.getContext('2d');

  sink.onframe = outputI420Frame => {
    const outputArgb32 = new Uint8ClampedArray(outputI420Frame.width * outputI420Frame.height * 4);
    i420ToArgb32(outputI420Frame, outputArgb32);
    const outputImageData = createImageData(outputArgb32, outputI420Frame.width, outputI420Frame.height);
    const outputCanvas = createCanvas(outputI420Frame.width, outputI420Frame.height);
    const outputContext = outputCanvas.getContext('2d');
    outputContext.putImageData(outputImageData, 0, 0);
    readTimestamp(outputContext);
    // const timestamp = readTimestamp(outputContext);
    // const lag = performance.now() - timestamp;
    // console.log(`${lag} ms`);
  };

  for (let i = 0; i < 30; i++) {
    printTimestamp(inputContext);
    const { data: inputArgb32Data } = inputContext.getImageData(0, 0, inputWidth, inputHeight);
    const inputI420Frame = I420Frame.fromArgb32(inputArgb32Data, inputWidth, inputHeight);
    source.onFrame(inputI420Frame);
    await new Promise(resolve => setTimeout(resolve));
  }

  sink.stop();
  track.stop();

  t.end();
});
