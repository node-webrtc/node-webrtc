'use strict';

const tape = require('tape');

const Connection = require('../../../../lib/server/connections/connection');
const WebRtcConnectionManager = require('../../../../lib/server/connections/webrtcconnectionmanager');

const LOCAL_DESCRIPTION = { type: 'offer', sdp: 'offer' };

class TestWebRtcConnection extends Connection {
  constructor(id) {
    super(id);
    this.localDescription = null;
    this.remoteDescription = null;
  }

  async doOffer() {
    this.localDescription = LOCAL_DESCRIPTION;
  }

  async applyAnswer(answer) {
    this.remoteDescription = answer;
  }

  toJSON() {
    return {
      ...super.toJSON(),
      localDescription: this.localDescription,
      remoteDescription: this.remoteDescription
    };
  }
}

tape('WebRtcConnectionManager', t => {
  t.test('constructor', t => {
    const connectionManager = new WebRtcConnectionManager();
    t.ok(connectionManager instanceof WebRtcConnectionManager, 'returns an instance of WebRtcConnectionManager');
    t.end();
  });

  t.test('.createConnection()', async t => {
    const connectionManager = new WebRtcConnectionManager({ Connection: TestWebRtcConnection });
    const connection = await connectionManager.createConnection();
    t.ok(connection instanceof TestWebRtcConnection, 'returns an instance of WebRtcConnection');
    t.deepEqual(connection.localDescription, LOCAL_DESCRIPTION, 'the WebRtcConnection has its .localDescription set');
    t.end();
  });

  t.end();
});
