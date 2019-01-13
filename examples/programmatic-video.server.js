/* eslint no-cond-assign:0, no-console:0 */
'use strict';

const express = require('express');
const browserify = require('browserify-middleware');
const { createCanvas, createImageData } = require('canvas');
const { hsv } = require('color-space');
const { createServer } = require('http');
const { join } = require('path');
const { performance } = require('perf_hooks');
const { Server } = require('ws');

const { RTCPeerConnection, RTCVideoSink, RTCVideoSource, i420ToArgb32 } = require('..');
const { I420Frame } = require('../test/lib/frame');
const { getOffer, onCandidate } = require('./loopback.common');

const app = express();

app.get('/programmatic-video.client.js', browserify(join(__dirname, 'programmatic-video.client.js')));

app.use(express.static(__dirname));

const server = createServer(app);

server.listen(8080, () => {
  const address = server.address();
  console.log(`Server running at ${address.port}`);
});

let i = 0;

new Server({ server }).on('connection', async ws => {
  const n = i++;

  console.log(`${n}: Creating new RTCPeerConnection`);

  const pc = new RTCPeerConnection({
    bundlePolicy: 'max-bundle',
    rtcpMuxPolicy: 'require'
  });

  let sink = null;
  const source = new RTCVideoSource();
  const track = source.createTrack();
  pc.addTrack(track);

  const width = 640;
  const height = 480;
  const canvas = createCanvas(width, height);
  const context = canvas.getContext('2d');
  context.fillStyle = 'white';
  context.fillRect(0, 0, width, height);

  let h = 0;
  let lastFrame = null;

  const interval = setInterval(() => {
    if (pc.signalingState === 'closed') {
      clearInterval(interval);
      return;
    }

    const thisTime = performance.now();
    context.fillStyle = 'rgba(255, 255, 255, 0.025)';

    if (lastFrame) {
      const lastFrameCanvas = createCanvas(lastFrame.width, lastFrame.height);
      const lastFrameContext = lastFrameCanvas.getContext('2d');

      const argb32 = new Uint8ClampedArray(lastFrame.width * lastFrame.height * 4);
      i420ToArgb32(lastFrame, argb32);

      const lastFrameImageData = createImageData(argb32, lastFrame.width, lastFrame.height);
      lastFrameContext.putImageData(lastFrameImageData, 0, 0);
      console.log(lastFrame.width, lastFrame.height);
      context.drawImage(lastFrameCanvas, 0, 0); // , lastFrame.width, lastFrame.height, 0, 0, width, height);
    } else {
      context.fillRect(0, 0, width, height);
    }

    h = (h + 1) % 360;
    const [r, g, b] = hsv.rgb([h, 100, 100]);

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

    const argb32 = canvas.toBuffer('raw');
    const frame = I420Frame.fromArgb32(argb32, width, height);
    source.onFrame(frame);
  });

  pc.onicecandidate = ({ candidate }) => {
    if (candidate) {
      console.log(`${n}: Sending ICE candidate`);
      ws.send(JSON.stringify({
        type: 'candidate',
        candidate
      }));
    }
  };

  let queuedCandidates = [];
  onCandidate(ws, async candidate => {
    if (!pc.remoteDescription) {
      queuedCandidates.push(candidate);
      return;
    }
    console.log(`${n}: Adding ICE candidate`);
    await pc.addIceCandidate(candidate);
    console.log(`${n}: Added ICE candidate`);
  });

  ws.once('close', () => {
    console.log(`${n}: Closing RTCPeerConnection`);
    pc.close();
    if (sink) {
      sink.stop();
    }
  });

  try {
    console.log(`${n}: Waiting for offer`);
    const offer = await getOffer(ws);

    console.log(`${n}: Received offer; setting remote description`);
    await pc.setRemoteDescription(offer);

    const remoteVideoTrack = pc.getReceivers()
      .map(receiver => receiver.track)
      .find(track => track.kind === 'video');
    sink = new RTCVideoSink(remoteVideoTrack);
    sink.onframe = frame => { lastFrame = frame; };

    console.log(`${n}: Set remote description; creating answer`);
    const answer = await pc.createAnswer();

    console.log(`${n}: Created answer; setting local description`);
    await pc.setLocalDescription(answer);

    console.log(`${n}: Set local description; sending answer`);
    ws.send(JSON.stringify(answer));

    await Promise.all(queuedCandidates.splice(0).map(async candidate => {
      console.log(`${n}: Adding ICE candidate`);
      await pc.addIceCandidate(candidate);
      console.log(`${n}: Added ICE candidate`);
    }));
  } catch (error) {
    console.error(error.stack || error.message || error);
    ws.close();
  }
});
