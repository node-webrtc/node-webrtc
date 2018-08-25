/* eslint no-cond-assign:0, no-console:0 */
'use strict';

const express = require('express');
const browserify = require('browserify-middleware');
const { createServer } = require('http');
const { join } = require('path');
const { Server } = require('ws');

const { RTCPeerConnection } = require('..');
const { getOffer, onCandidate } = require('./loopback.common');

const app = express();

app.get('/loopback.client.js', browserify(join(__dirname, 'loopback.client.js')));

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

  pc.onicecandidate = ({ candidate }) => {
    if (candidate) {
      console.log(`${n}: Sending ICE candidate`);
      ws.send(JSON.stringify({
        type: 'candidate',
        candidate
      }));
    }
  };

  pc.ontrack = ({ track, streams }) => {
    console.log(`${n}: Received ${track.kind} MediaStreamTrack with ID ${track.id}`);
    pc.addTrack(track, ...streams);
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
