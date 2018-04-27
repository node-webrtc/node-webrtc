'use strict';

var _webrtc = require('./binding');

var EventTarget = require('./eventtarget');

var RTCDataChannel = require('./datachannel');
var RTCDataChannelEvent = require('./datachannelevent');
var RTCIceCandidate = require('./icecandidate');
var RTCPeerConnectionIceEvent = require('./rtcpeerconnectioniceevent');
var RTCSessionDescription = require('./sessiondescription');
var RTCStatsResponse = require('./rtcstatsresponse');

function RTCPeerConnection() {
  var self = this;
  var pc = new _webrtc.PeerConnection(arguments[0] || {});

  EventTarget.call(this);

  //
  // Attach events to the native PeerConnection object
  //
  pc.ontrack = function ontrack(receiver, streams) {
    self.dispatchEvent({
      type: 'track',
      track: receiver.track,
      receiver: receiver,
      streams: streams
    });
  };

  pc.onconnectionstatechange = function onconnectionstatechange() {
    self.dispatchEvent({ type: 'connectionstatechange' });
  };

  pc.onicecandidate = function onicecandidate(candidate) {
    var icecandidate = new RTCIceCandidate(candidate);
    self.dispatchEvent(new RTCPeerConnectionIceEvent('icecandidate', { candidate: icecandidate }));
  };

  pc.onsignalingstatechange = function onsignalingstatechange() {
    self.dispatchEvent({ type: 'signalingstatechange' });
  };

  pc.oniceconnectionstatechange = function oniceconnectionstatechange() {
    self.dispatchEvent({ type: 'iceconnectionstatechange' });
  };

  pc.onicegatheringstatechange = function onicegatheringstatechange() {
    self.dispatchEvent({ type: 'icegatheringstatechange' });

    // if we have completed gathering candidates, trigger a null candidate event
    if (self.iceGatheringState === 'complete') {
      self.dispatchEvent(new RTCPeerConnectionIceEvent('icecandidate', { candidate: null }));
    }
  };

  pc.onnegotiationneeded = function onnegotiationneeded() {
    self.dispatchEvent({ type: 'negotiationneeded' });
  };

  // [ToDo] onnegotiationneeded

  pc.ondatachannel = function ondatachannel(internalDC) {
    var dc = new RTCDataChannel(internalDC);

    self.dispatchEvent(new RTCDataChannelEvent('datachannel', { channel: dc }));
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
      }
    },
    connectionState: {
      get: function getConnectionState() {
        return pc.connectionState;
      }
    },
    currentLocalDescription: {
      get: function getCurrentLocalDescription() {
        return pc.currentLocalDescription
          ? new RTCSessionDescription(pc.currentLocalDescription)
          : null;
      }
    },
    localDescription: {
      get: function getLocalDescription() {
        return pc.localDescription
          ? new RTCSessionDescription(pc.localDescription)
          : null;
      }
    },
    pendingLocalDescription: {
      get: function getPendingLocalDescription() {
        return pc.pendingLocalDescription
          ? new RTCSessionDescription(pc.pendingLocalDescription)
          : null;
      }
    },
    currentRemoteDescription: {
      get: function getCurrentRemoteDescription() {
        return pc.currentRemoteDescription
          ? new RTCSessionDescription(pc.currentRemoteDescription)
          : null;
      }
    },
    remoteDescription: {
      get: function getRemoteDescription() {
        return pc.remoteDescription
          ? new RTCSessionDescription(pc.remoteDescription)
          : null;
      }
    },
    pendingRemoteDescription: {
      get: function getPendingRemoteDescription() {
        return pc.pendingRemoteDescription
          ? new RTCSessionDescription(pc.pendingRemoteDescription)
          : null;
      }
    },
    signalingState: {
      get: function getSignalingState() {
        return pc.signalingState;
      }
    },
    readyState: {
      get: function getReadyState() {
        return pc.getReadyState();
      }
    },
    iceGatheringState: {
      get: function getIceGatheringState() {
        return pc.iceGatheringState;
      }
    },
    iceConnectionState: {
      get: function getIceConnectionState() {
        return pc.iceConnectionState;
      }
    },
    onconnectionstatechange: {
      value: null,
      writable: true
    },
    ondatachannel: {
      value: null,
      writable: true
    },
    oniceconnectionstatechange: {
      value: null,
      writable: true
    },
    onicegatheringstatechange: {
      value: null,
      writable: true
    },
    onnegotiationneeded: {
      value: null,
      writable: true
    },
    onsignalingstatechange: {
      value: null,
      writable: true
    }
  });
}

RTCPeerConnection.prototype.addIceCandidate = function addIceCandidate(candidate) {
  var promise = this._pc.addIceCandidate(candidate);
  if (arguments.length === 3) {
    promise.then(arguments[1], arguments[2]);
  }
  return promise;
};

RTCPeerConnection.prototype.addTrack = function addTrack(track, stream) {
  return this._pc.addTrack(track, stream);
};

RTCPeerConnection.prototype.close = function close() {
  this._pc.close();
};

RTCPeerConnection.prototype.createDataChannel = function createDataChannel() {
  try {
    return new RTCDataChannel(this._pc.createDataChannel.apply(this._pc, arguments));
  } catch (error) {
    // TODO(mroberts): Start using the domexception library.
    if (error.message.match(/signalingState is 'closed'/)) {
      error.code = 11;
      error.name = 'InvalidStateError';
    }
    throw error;
  }
};

RTCPeerConnection.prototype.createOffer = function createOffer() {
  var options = arguments.length === 3
    ? arguments[2]
    : arguments[0];
  var promise = this._pc.createOffer(options || {}).then(function(init) {
    return new RTCSessionDescription(init);
  });
  if (arguments.length >= 2) {
    promise.then(arguments[0], arguments[1]);
  }
  return promise;
};

RTCPeerConnection.prototype.createAnswer = function createAnswer() {
  var options = arguments.length === 3
    ? arguments[2]
    : arguments[0];
  var promise = this._pc.createAnswer(options || {}).then(function(init) {
    return new RTCSessionDescription(init);
  });
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

// NOTE(mroberts): simple-peer is doing feature detection based on the number of
// arguments this method takes. Don't change this until we can support the
// proper standards-based `getStats` API.
RTCPeerConnection.prototype.getStats = function getStats(onSuccess, onFailure) {
  this._pc.getStats().then(function(internalRTCStatsResponse) {
    return new RTCStatsResponse(internalRTCStatsResponse);
  }).then(onSuccess, onFailure);
};

RTCPeerConnection.prototype.removeTrack = function removeTrack(sender) {
  this._pc.removeTrack(sender);
};

RTCPeerConnection.prototype.setConfiguration = function setConfiguration(configuration) {
  try {
    return this._pc.setConfiguration(configuration);
  } catch (error) {
    // TODO(mroberts): Start using the domexception library.
    if (error.message.match(/InvalidModificationError/)) {
      error.code = 13;
      error.name = 'InvalidModificationError';
    }
    throw error;
  }
};

RTCPeerConnection.prototype.setLocalDescription = function setLocalDescription(description) {
  var promise = this._pc.setLocalDescription(description).catch(function(error) {
    // TODO(mroberts): Start using the domexception library.
    if (error.message.match(/Called in wrong state:/)) {
      error.code = 11;
      error.name = 'InvalidStateError';
    } else if (error.message.match(/does not match/)) {
      error.code = 13;
      error.name = 'InvalidModificationError';
    }
    throw error;
  });
  if (arguments.length === 3) {
    promise.then(arguments[1], arguments[2]);
  }
  return promise;
};

RTCPeerConnection.prototype.setRemoteDescription = function setRemoteDescription(description) {
  var promise = this._pc.setRemoteDescription(description).catch(function(error) {
    // TODO(mroberts): Start using the domexception library.
    if (error.message.match(/Called in wrong state:/)) {
      error.code = 11;
      error.name = 'InvalidStateError';
    }
    throw error;
  });
  if (arguments.length === 3) {
    promise.then(arguments[1], arguments[2]);
  }
  return promise;
};

module.exports = RTCPeerConnection;
