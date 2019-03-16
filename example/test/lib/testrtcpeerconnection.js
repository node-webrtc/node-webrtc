'use strict';

const EventEmitter = require('events');

const LOCAL_DESCRIPTION = { type: 'offer', sdp: 'offer' };

class TestRtcDescription {
  constructor(init) {
    this.type = init.type;
    this.sdp = init.sdp;
  }

  toJSON() {
    return this;
  }
}

class TestRtcPeerConnection extends EventEmitter {
  constructor() {
    super();
    this.iceConnectionState = 'new';
    this.signalingState = 'stable';
    this.localDescription = null;
    this.remoteDescription = null;
    TestRtcPeerConnection.peerConnections.push(this);
  }

  close() {
    this.signalingState = 'closed';
  }

  createDataChannel() {
    return {};
  }

  async createOffer() {
    return LOCAL_DESCRIPTION;
  }

  async setLocalDescription(localDescription) {
    this.localDescription = new TestRtcDescription(localDescription);
    this.signalingState = 'have-local-offer';
    setTimeout(() => {
      this.emit('icecandidate', { candidate: null });
    });
  }

  async setRemoteDescription(remoteDescription) {
    this.remoteDescription = new TestRtcDescription(remoteDescription);
    this.signalingState = 'stable';
  }

  addEventListener(event, listener) {
    this.on(event, listener);
  }

  removeEventListener(event, listener) {
    this.removeListener(event, listener);
  }

  updateIceConnectionState(iceConnectionState) {
    this.iceConnectionState = iceConnectionState;
    this.emit('iceconnectionstatechange');
  }
}

TestRtcPeerConnection.peerConnections = [];

module.exports = TestRtcPeerConnection;
