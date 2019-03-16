'use strict';

const bodyParser = require('body-parser');
const express = require('express');
const tape = require('tape');

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

  t.end();
});
