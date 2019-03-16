'use strict';

const { createCanvas, createImageData } = require('canvas');
const { hsv } = require('color-space');
const { performance } = require('perf_hooks');

const { RTCVideoSink, RTCVideoSource, i420ToRgba } = require('../../..').nonstandard;

// TODO(mroberts): Do not depend on this.
const { I420Frame } = require('../../../test/lib/frame');

const width = 640;
const height = 480;

function beforeOffer(peerConnection) {
  const source = new RTCVideoSource();
  const track = source.createTrack();
  const transceiver = peerConnection.addTransceiver(track);
  const sink = new RTCVideoSink(transceiver.receiver.track);

  let lastFrame = null;

  function onFrame({ frame }) {
    lastFrame = frame;
  }

  sink.addEventListener('frame', onFrame);

  // TODO(mroberts): Is pixelFormat really necessary?
  const canvas = createCanvas(width, height);
  const context = canvas.getContext('2d', { pixelFormat: 'RGBA24' });
  context.fillStyle = 'white';
  context.fillRect(0, 0, width, height);

  let hue = 0;

  const interval = setInterval(() => {
    if (lastFrame) {
      const lastFrameCanvas = createCanvas(lastFrame.width,  lastFrame.height);
      const lastFrameContext = lastFrameCanvas.getContext('2d', { pixelFormat: 'RGBA24' });

      const rgba = new Uint8ClampedArray(lastFrame.width *  lastFrame.height * 4);
      const rgbaFrame = createImageData(rgba, lastFrame.width, lastFrame.height);
      i420ToRgba(lastFrame, rgbaFrame);

      lastFrameContext.putImageData(rgbaFrame, 0, 0);
      context.drawImage(lastFrameCanvas, 0, 0);
    } else {
      context.fillStyle = 'rgba(255, 255, 255, 0.025)';
      context.fillRect(0, 0, width, height);
    }

    hue = ++hue % 360;
    const [r, g, b] = hsv.rgb([hue, 100, 100]);
    const thisTime = performance.now();

    context.font = '60px Sans-serif';
    context.strokeStyle = 'black';
    context.lineWidth = 1;
    context.fillStyle = `rgba(${Math.round(r)}, ${Math.round(g)}, ${Math.round(b)}, 1)`;
    context.textAlign = 'center';
    context.save();
    context.translate(width / 2, height / 2);
    context.rotate(thisTime / 1000);
    context.strokeText('node-webrtc', 0, 0);
    context.fillText('node-webrtc', 0, 0);
    context.restore();

    const rgbaFrame = context.getImageData(0, 0, width, height);
    const i420Frame = I420Frame.fromRgba(rgbaFrame);
    source.onFrame(i420Frame);
  });

  // NOTE(mroberts): This is a hack so that we can get a callback when the
  // RTCPeerConnection is closed. In the future, we can subscribe to
  // "connectionstatechange" events.
  const { close } = peerConnection;
  peerConnection.close = function() {
    clearInterval(interval);
    sink.stop();
    track.stop();
    return close.apply(this, arguments);
  };
}

module.exports = { beforeOffer };
