/* eslint no-cond-assign:0, no-console:0 */
'use strict';

const express = require('express');
const browserify = require('browserify-middleware');
const { createServer } = require('http');
const { join } = require('path');
const { performance } = require('perf_hooks');
const { Server } = require('ws');

const { RTCPeerConnection, RTCVideoSource } = require('..');
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

  const source = new RTCVideoSource();
  const track = source.createTrack();
  pc.addTrack(track);

  const width = 640;
  const height = 480;
  const sizeOfLuminancePlane = width * height;
  const sizeOfChromaPlane = sizeOfLuminancePlane / 4;
  const byteLength
    = sizeOfLuminancePlane  // Y
    + sizeOfChromaPlane     // U
    + sizeOfChromaPlane;    // V
  const data = new Uint8ClampedArray(byteLength);
  for (let i = 0; i < sizeOfChromaPlane; i++) {
    data[sizeOfLuminancePlane + i] = 127;
    data[sizeOfLuminancePlane + sizeOfChromaPlane + i] = 127;
  }
  const frame = {
    width,
    height,
    data
  };

  const f1 = 0.00025;

  const interval = setInterval(() => {
    if (pc.signalingState === 'closed') {
      clearInterval(interval);
      return;
    }

    const thisTime = performance.now();

    const f2 = 0.01 * Math.sin(f1 * 50 * thisTime) + 0.5;
    for (let i = 0; i < width * height; i++) {
      data[i] = (Math.sin(f1 * f2 * i) + 0.5) * 255;
    }
    const shade = (Math.sin(f1 * 10 * thisTime) + 0.5) * 255;
    for (let i = 0; i < sizeOfChromaPlane; i++) {
      data[sizeOfLuminancePlane + i] = shade;
      data[sizeOfLuminancePlane + sizeOfChromaPlane + i] = f2 * shade;
    }

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
  });

  try {
    console.log(`${n}: Waiting for offer`);
    const offer = await getOffer(ws);

    console.log(`${n}: Received offer; setting remote description`);
    await pc.setRemoteDescription(offer);

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
