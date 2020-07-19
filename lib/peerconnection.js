'use strict';

var inherits = require('util').inherits;

var _webrtc = require('./binding');

var EventTarget = require('./eventtarget');

var RTCDataChannelEvent = require('./datachannelevent');
var RTCIceCandidate = require('./icecandidate');
var RTCPeerConnectionIceEvent = require('./rtcpeerconnectioniceevent');
var RTCPeerConnectionIceErrorEvent = require('./rtcpeerconnectioniceerrorevent');
var RTCSessionDescription = require('./sessiondescription');

function RTCPeerConnection() {
  var self = this;
  var pc = new _webrtc.RTCPeerConnection(arguments[0] || {});

  EventTarget.call(this);

  //
  // Attach events to the native PeerConnection object
  //
  pc.ontrack = function ontrack(receiver, streams, transceiver) {
    self.dispatchEvent({
      type: 'track',
      track: receiver.track,
      receiver: receiver,
      streams: streams,
      transceiver: transceiver
    });
  };

  pc.onconnectionstatechange = function onconnectionstatechange() {
    self.dispatchEvent({ type: 'connectionstatechange' });
  };

  pc.onicecandidate = function onicecandidate(candidate) {
    var icecandidate = new RTCIceCandidate(candidate);
    self.dispatchEvent(new RTCPeerConnectionIceEvent('icecandidate', { candidate: icecandidate, target: self }));
  };

  pc.onicecandidateerror = function onicecandidateerror(eventInitDict) {
    var pair = eventInitDict.hostCandidate.split(':');
    eventInitDict.address = pair[0];
    eventInitDict.port = pair[1];
    var icecandidateerror = new RTCPeerConnectionIceErrorEvent('icecandidateerror', eventInitDict);
    self.dispatchEvent(icecandidateerror);
  };

  pc.onsignalingstatechange = function onsignalingstatechange() {
    self.dispatchEvent({ type: 'signalingstatechange', target: self });
  };

  pc.oniceconnectionstatechange = function oniceconnectionstatechange() {
    self.dispatchEvent({ type: 'iceconnectionstatechange', target: self });
  };

  pc.onicegatheringstatechange = function onicegatheringstatechange() {
    self.dispatchEvent({ type: 'icegatheringstatechange', target: self });

    // if we have completed gathering candidates, trigger a null candidate event
    if (self.iceGatheringState === 'complete' && self.connectionState !== 'closed') {
      self.dispatchEvent(new RTCPeerConnectionIceEvent('icecandidate', { candidate: null, target: self }));
    }
  };

  pc.onnegotiationneeded = function onnegotiationneeded() {
    self.dispatchEvent({ type: 'negotiationneeded', target: self });
  };

  // [ToDo] onnegotiationneeded

  pc.ondatachannel = function ondatachannel(channel) {
    self.dispatchEvent(new RTCDataChannelEvent('datachannel', { channel }));
  };

  //
  // PeerConnection properties & attributes
  //

  Object.defineProperties(this, {
    _pc: {
      value: pc
    },
    canTrickleIceCandidates: {
      get: function getCanTrickleIceCandidates() {
        return pc.canTrickleIceCandidates;
      },
      enumerable: true
    },
    connectionState: {
      get: function getConnectionState() {
        return pc.connectionState;
      },
      enumerable: true
    },
    currentLocalDescription: {
      get: function getCurrentLocalDescription() {
        return pc.currentLocalDescription
          ? new RTCSessionDescription(pc.currentLocalDescription)
          : null;
      },
      enumerable: true
    },
    localDescription: {
      get: function getLocalDescription() {
        return pc.localDescription
          ? new RTCSessionDescription(pc.localDescription)
          : null;
      },
      enumerable: true
    },
    pendingLocalDescription: {
      get: function getPendingLocalDescription() {
        return pc.pendingLocalDescription
          ? new RTCSessionDescription(pc.pendingLocalDescription)
          : null;
      },
      enumerable: true
    },
    currentRemoteDescription: {
      get: function getCurrentRemoteDescription() {
        return pc.currentRemoteDescription
          ? new RTCSessionDescription(pc.currentRemoteDescription)
          : null;
      },
      enumerable: true
    },
    remoteDescription: {
      get: function getRemoteDescription() {
        return pc.remoteDescription
          ? new RTCSessionDescription(pc.remoteDescription)
          : null;
      },
      enumerable: true
    },
    pendingRemoteDescription: {
      get: function getPendingRemoteDescription() {
        return pc.pendingRemoteDescription
          ? new RTCSessionDescription(pc.pendingRemoteDescription)
          : null;
      },
      enumerable: true
    },
    signalingState: {
      get: function getSignalingState() {
        return pc.signalingState;
      },
      enumerable: true
    },
    readyState: {
      get: function getReadyState() {
        return pc.getReadyState();
      }
    },
    sctp: {
      get: function() {
        return pc.sctp;
      },
      enumerable: true
    },
    iceGatheringState: {
      get: function getIceGatheringState() {
        return pc.iceGatheringState;
      },
      enumerable: true
    },
    iceConnectionState: {
      get: function getIceConnectionState() {
        return pc.iceConnectionState;
      },
      enumerable: true
    },
    onconnectionstatechange: {
      value: null,
      writable: true,
      enumerable: true
    },
    ondatachannel: {
      value: null,
      writable: true,
      enumerable: true
    },
    oniceconnectionstatechange: {
      value: null,
      writable: true,
      enumerable: true
    },
    onicegatheringstatechange: {
      value: null,
      writable: true,
      enumerable: true
    },
    onnegotiationneeded: {
      value: null,
      writable: true,
      enumerable: true
    },
    onsignalingstatechange: {
      value: null,
      writable: true,
      enumerable: true
    }
  });
}

inherits(RTCPeerConnection, EventTarget);

// NOTE(mroberts): This is a bit of a hack.
RTCPeerConnection.prototype.ontrack = null;

RTCPeerConnection.prototype.addIceCandidate = function addIceCandidate(candidate) {
  var promise = this._pc.addIceCandidate(candidate);
  if (arguments.length === 3) {
    promise.then(arguments[1], arguments[2]);
  }
  return promise;
};

RTCPeerConnection.prototype.addTransceiver = function addTransceiver() {
  return this._pc.addTransceiver.apply(this._pc, arguments);
};

RTCPeerConnection.prototype.addTrack = function addTrack(track, ...streams) {
  return this._pc.addTrack(track, streams);
};

RTCPeerConnection.prototype.close = function close() {
  this._pc.close();
};

RTCPeerConnection.prototype.createDataChannel = function createDataChannel() {
  return this._pc.createDataChannel.apply(this._pc, arguments);
};

RTCPeerConnection.prototype.createOffer = function createOffer() {
  var options = arguments.length === 3
    ? arguments[2]
    : arguments[0];
  var promise = this._pc.createOffer(options || {});
  if (arguments.length >= 2) {
    promise.then(arguments[0], arguments[1]);
  }
  return promise;
};

RTCPeerConnection.prototype.createAnswer = function createAnswer() {
  var options = arguments.length === 3
    ? arguments[2]
    : arguments[0];
  var promise = this._pc.createAnswer(options || {});
  if (arguments.length >= 2) {
    promise.then(arguments[0], arguments[1]);
  }
  return promise;
};

RTCPeerConnection.prototype.getConfiguration = function getConfiguration() {
  return this._pc.getConfiguration();
};

RTCPeerConnection.prototype.getReceivers = function getReceivers() {
  return this._pc.getReceivers();
};

RTCPeerConnection.prototype.getSenders = function getSenders() {
  return this._pc.getSenders();
};

RTCPeerConnection.prototype.getTransceivers = function getTransceivers() {
  return this._pc.getTransceivers();
};

RTCPeerConnection.prototype.getStats = function getStats() {
  if (typeof arguments[0] === 'function') {
    this._pc.legacyGetStats().then(arguments[0], arguments[1]);
    return;
  }
  return this._pc.getStats();
};

RTCPeerConnection.prototype.removeTrack = function removeTrack(sender) {
  this._pc.removeTrack(sender);
};

RTCPeerConnection.prototype.setConfiguration = function setConfiguration(configuration) {
  return this._pc.setConfiguration(configuration);
};

RTCPeerConnection.prototype.setLocalDescription = function setLocalDescription(description) {
  var promise = this._pc.setLocalDescription(description);
  if (arguments.length === 3) {
    promise.then(arguments[1], arguments[2]);
  }
  return promise;
};

RTCPeerConnection.prototype.setRemoteDescription = function setRemoteDescription(description) {
  var promise = this._pc.setRemoteDescription(description);
  if (arguments.length === 3) {
    promise.then(arguments[1], arguments[2]);
  }
  return promise;
};

RTCPeerConnection.prototype.restartIce = function restartIce() {
  return this._pc.restartIce();
};

module.exports = RTCPeerConnection;
