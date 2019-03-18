'use strict';

const bodyParser = require('body-parser');
const express = require('express');
const tape = require('tape');

const sineWaveExampleOptions = require('../../../examples/sine-wave/server.js');
const ConnectionClient = require('../../../lib/client');
const WebRtcConnectionManager = require('../../../lib/server/connections/webrtcconnectionmanager');
const connectionsApi = require('../../../lib/server/rest/connectionsapi');

tape('ConnectionsClient', t => {
  t.test('typical usage', t => {
    const app = express();

    app.use(bodyParser.json());

    const connectionManager = WebRtcConnectionManager.create({
      beforeOffer(peerConnection) {
        peerConnection.createDataChannel('test');
      },
      timeToReconnected: 0
    });

    connectionsApi(app, connectionManager);

    const server = app.listen(3000, async () => {
      const connectionClient = new ConnectionClient({
        host: 'http://localhost:3000',
        prefix: '/v1'
      });

      const peerConnection = await connectionClient.createConnection();

      peerConnection.close();

      connectionManager.getConnections().forEach(connection => connection.close());

      server.close();

      t.end();
    });
  });

  t.test('stress test', t => {
    const app = express();

    app.use(bodyParser.json());

    const connectionManager = WebRtcConnectionManager.create(sineWaveExampleOptions);

    connectionsApi(app, connectionManager);

    const server = app.listen(3000, async () => {
      const peerConnections = [];

      for (let i = 0; i < 100; i++) {
        const connectionClient = new ConnectionClient({
          host: 'http://localhost:3000',
          prefix: '/v1'
        });

        const peerConnection = await connectionClient.createConnection();
        peerConnections.push(peerConnection);
      }

      await delay(3000);

      peerConnections.forEach(peerConnection => peerConnection.close());

      connectionManager.getConnections().forEach(connection => connection.close());

      server.close();

      t.end();
    });
  });

  t.end();
});

function delay(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}
