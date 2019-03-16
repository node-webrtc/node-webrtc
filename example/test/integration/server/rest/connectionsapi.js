'use strict';

const bodyParser = require('body-parser');
const express = require('express');
const fetch = require('node-fetch');
const tape = require('tape');

const WebRtcConnection = require('../../../../lib/server/connections/webrtcconnection');
const WebRtcConnectionManager = require('../../../../lib/server/connections/webrtcconnectionmanager');
const connectionsApi = require('../../../../lib/server/rest/connectionsapi');

const TestRtcPeerConnection = require('../../../lib/testrtcpeerconnection');

async function delete_(url) {
  const response = await fetch(url, {
    method: 'delete'
  });
  return response.json();
}

async function get(url) {
  const response = await fetch(url);
  return response.json();
}

async function post(url, body) {
  const options = {
    headers: {},
    method: 'post'
  };
  if (typeof body !== 'undefined') {
    options.body = JSON.stringify(body);
    options.headers['Content-Type'] = 'application/json';
  }
  const response = await fetch(url, options);
  return response.json();
}

tape('connectionsApi(app, connectionManager)', t => {
  t.test('typical usage', t => {
    const app = express();

    app.use(bodyParser.json());

    const connectionManager = new WebRtcConnectionManager({
      Connection: function Connection(id) {
        return new WebRtcConnection(id, {
          RTCPeerConnection: TestRtcPeerConnection
        });
      }
    });

    connectionsApi(app, connectionManager);

    const server = app.listen(3000, async () => {
      const connections1 = await get('http://localhost:3000/v1/connections');

      t.deepEqual(connections1, [], 'GET /v1/connections initially returns the empty array');

      const connection1 = await post('http://localhost:3000/v1/connections');

      TestRtcPeerConnection.peerConnections.shift();

      t.ok(connection1.localDescription, 'POST /v1/connections returns a new connection with a local description');

      const connection2 = await get(`http://localhost:3000/v1/connections/${connection1.id}`);

      t.deepEqual(connection1, connection2, 'GET /v1/connection/$id returns the connection');

      const connections2 = await get('http://localhost:3000/v1/connections');

      t.deepEqual(connections2, connectionManager.toJSON(), 'GET /v1/connections later returns the updated array');

      const localDescription = await get(`http://localhost:3000/v1/connections/${connection2.id}/local-description`);

      t.deepEqual(localDescription, connection2.localDescription, 'GET /v1/connections/$id/local-description returns the .localDescription');

      const remoteDescription1 = await get(`http://localhost:3000/v1/connections/${connection2.id}/remote-description`);

      t.deepEqual(remoteDescription1, {}, 'GET /v1/connections/$id/remote-description returns the .remoteDescription');

      const remoteDescription2 = await post(`http://localhost:3000/v1/connections/${connection2.id}/remote-description`, {
        type: 'answer',
        sdp: 'answer'
      });

      t.deepEqual(remoteDescription2, { type: 'answer', sdp: 'answer' }, 'POST /v1/connections/$id/remote-description allows updating the .remoteDescription');

      const remoteDescription3 = await get(`http://localhost:3000/v1/connections/${connection2.id}/remote-description`);

      t.deepEqual(remoteDescription3, remoteDescription2, 'GET /v1/connections/$id/remote-description returns the updated .remoteDescription');

      const connection3 = await delete_(`http://localhost:3000/v1/connections/${connection1.id}`);

      t.equal(connection3.state, 'closed', 'DELETE /v1/connection/$id returns the closed WebRtcConnection');

      const connections3 = await get('http://localhost:3000/v1/connections');

      t.deepEqual(connections3, [], 'GET /v1/connections finally returns the empty array again');

      server.close();
      t.end();
    });
  });

  t.end();
});
