'use strict';

const fetch = require('node-fetch');
const DefaultRTCPeerConnection = require('../../..').RTCPeerConnection;

const TIME_TO_HOST_CANDIDATES = 3000;  // NOTE(mroberts): Too long.

class ConnectionClient {
  constructor(options = {}) {
    options = {
      RTCPeerConnection: DefaultRTCPeerConnection,
      clearTimeout,
      host: '',
      prefix: '.',
      setTimeout,
      timeToHostCandidates: TIME_TO_HOST_CANDIDATES,
      ...options
    };

    const {
      RTCPeerConnection,
      prefix,
      host
    } = options;

    this.createConnection = async (options = {}) => {
      options = {
        beforeAnswer() {},
        ...options
      };

      const {
        beforeAnswer
      } = options;

      const response1 = await fetch(`${host}${prefix}/connections`, {
        method: 'POST'
      });

      const remotePeerConnection = await response1.json();
      const { id } = remotePeerConnection;

      const localPeerConnection = new RTCPeerConnection({
        sdpSemantics: 'unified-plan'
      });

      // NOTE(mroberts): This is a hack so that we can get a callback when the
      // RTCPeerConnection is closed. In the future, we can subscribe to
      // "connectionstatechange" events.
      localPeerConnection.close = function() {
        fetch(`${host}${prefix}/connections/${id}`, { method: 'delete' }).catch(() => {});
        return RTCPeerConnection.prototype.close.apply(this, arguments);
      };

      try {
        await localPeerConnection.setRemoteDescription(remotePeerConnection.localDescription);

        await beforeAnswer(localPeerConnection);

        const answer = await localPeerConnection.createAnswer();
        await localPeerConnection.setLocalDescription(answer);

        // await waitUntilIceGatheringStateComplete(peerConnection, options);

        await fetch(`${host}${prefix}/connections/${id}/remote-description`, {
          method: 'POST',
          body: JSON.stringify(localPeerConnection.localDescription),
          headers: {
            'Content-Type': 'application/json'
          }
        });

        return localPeerConnection;
      } catch (error) {
        localPeerConnection.close();
        throw error;
      }
    };
  }
}

module.exports = ConnectionClient;
