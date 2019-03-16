'use strict';

const tape = require('tape');

const WebRtcConnection = require('../../../../lib/server/connections/webrtcconnection');

const TestRtcPeerConnection = require('../../../lib/testrtcpeerconnection');

const ID = 0;

tape('WebRtcConnection', t => {
  t.test('constructor', t => {
    const connection = new WebRtcConnection(ID, { RTCPeerConnection: TestRtcPeerConnection });
    TestRtcPeerConnection.peerConnections.shift();
    t.ok(connection instanceof WebRtcConnection, 'it returns an instance of WebRtcConnection');
    connection.close();
    t.end();
  });

  t.test('connection timer, if fired before .iceConnectionState is "connected" or "completed"', t => {
    const callbacks = [];

    const connection = new WebRtcConnection(ID, {
      RTCPeerConnection: TestRtcPeerConnection,
      setTimeout(callback) {
        callbacks.push(callback);
        return callback;
      },
      clearTimeout(callback) {
        const index = callbacks.indexOf(callback);
        if (index > -1) {
          callbacks.splice(index, 1);
        }
      }
    });

    const peerConnection = TestRtcPeerConnection.peerConnections.shift();

    callbacks[0]();

    t.equal(peerConnection.signalingState, 'closed', 'the RTCPeerConnection will be closed');
    t.equal(connection.state, 'closed', 'the WebRtcConnection will be closed');

    t.end();
  });

  t.test('if .iceConnectionState becomes "connected" or "completed" before the connection timer fires', t => {
    const callbacks = [];

    const connection = new WebRtcConnection(ID, {
      RTCPeerConnection: TestRtcPeerConnection,
      setTimeout(callback) {
        callbacks.push(callback);
        return callback;
      },
      clearTimeout(callback) {
        const index = callbacks.indexOf(callback);
        if (index > -1) {
          callbacks.splice(index, 1);
        }
      }
    });

    const peerConnection = TestRtcPeerConnection.peerConnections.shift();

    const [callback] = callbacks;
    peerConnection.updateIceConnectionState('connected');

    t.ok(!callbacks.includes(callback), 'the connection timer will be canceled');
    connection.close();

    t.end();
  });

  t.test('reconnection timer, if fired before .iceConnectionState is "connected" or "completed"', t => {
    const callbacks = [];

    const connection = new WebRtcConnection(ID, {
      RTCPeerConnection: TestRtcPeerConnection,
      setTimeout(callback) {
        callbacks.push(callback);
        return callback;
      },
      clearTimeout(callback) {
        const index = callbacks.indexOf(callback);
        if (index > -1) {
          callbacks.splice(index, 1);
        }
      }
    });

    const peerConnection = TestRtcPeerConnection.peerConnections.shift();
    peerConnection.updateIceConnectionState('connected');
    peerConnection.updateIceConnectionState('disconnected');

    callbacks[0]();

    t.equal(peerConnection.signalingState, 'closed', 'the RTCPeerConnection will be closed');
    t.equal(connection.state, 'closed', 'the WebRtcConnection will be closed');

    t.end();
  });

  t.test('if .iceConnectionState becomes "connected" or "completed" before the connection timer fires', t => {
    const callbacks = [];

    const connection = new WebRtcConnection(ID, {
      RTCPeerConnection: TestRtcPeerConnection,
      setTimeout(callback) {
        callbacks.push(callback);
        return callback;
      },
      clearTimeout(callback) {
        const index = callbacks.indexOf(callback);
        if (index > -1) {
          callbacks.splice(index, 1);
        }
      }
    });

    const peerConnection = TestRtcPeerConnection.peerConnections.shift();
    peerConnection.updateIceConnectionState('connected');
    peerConnection.updateIceConnectionState('disconnected');

    const [callback] = callbacks;
    peerConnection.updateIceConnectionState('connected');

    t.ok(!callbacks.includes(callback), 'the reconnection timer will be canceled');
    connection.close();

    t.end();
  });

  t.end();
});
