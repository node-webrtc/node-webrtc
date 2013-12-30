var _webrtc = require('bindings')('webrtc.node');

var RTCSessionDescription = require('./sessiondescription');
var RTCIceCandidate = require('./icecandidate');
var RTCError = require('./error');
var RTCDataChannel = require('./datachannel');
var RTCMediaStream = require('./mediastream');


function PeerConnection(configuration, constraints) {
  var that = this;
  this._pc = new _webrtc.PeerConnection(configuration, constraints);

  this._localType = null;
  this._remoteType = null;

  this._queue = [];
  this._pending = null;

  this._dataChannels = {};  // open data channels, indexed by label

  this._pc.onerror = function onerror() {
    if(that._pending && that._pending.onError) {
      that._pending.onError.apply(that, arguments);
    }
    that._executeNext();
  };

  this._pc.onsuccess = function onsuccess() {
    if(that._pending && that._pending.onSuccess) {
      that._pending.onSuccess.apply(that, arguments);
    }
    that._executeNext();
  };

  this._pc.onicecandidate = function onicecandidate(candidate, sdpMid, sdpMLineIndex) {
    if(that.onicecandidate && typeof that.onicecandidate == 'function') {
      that.onicecandidate.apply(that, [new RTCIceCandidate({
        'candidate': candidate,
        'sdpMid': sdpMid,
        'sdpMLineIndex': sdpMLineIndex
      })]);
    }
  };

  this._pc.onsignalingstatechange = function onsignalingstatechange(state) {
    stateString = that.RTCSignalingStates[state];
    if('closed' == stateString) {
      Object.keys(that._dataChannels).forEach(function(label) {
        that._dataChannels[label].shutdown();
        delete that._dataChannels[label];
      });
    }
    if(that.onsignalingstatechange && typeof that.onsignalingstatechange == 'function') {
      that.onsignalingstatechange.apply(that, [stateString]);
    }
  };

  this._pc.oniceconnectionstatechange = function oniceconnectionstatechange(state) {
    if(that.oniceconnectionstatechange && typeof that.oniceconnectionstatechange == 'function') {
      stateString = that.RTCSignalingStates[state];
      that.oniceconnectionstatechange.apply(that, [stateString]);
    }
  };

  this._pc.onicegatheringstatechange = function onicegatheringstatechange(state) {
    if(that.onicegatheringstatechange && typeof that.onicegatheringstatechange == 'function') {
      stateString = that.RTCSignalingStates[state];
      that.onicegatheringstatechange.apply(that, [stateString]);
    }
  };

  this._pc.ondatachannel = function ondatachannel(internalDC) {
    if(that.ondatachannel && typeof that.ondatachannel == 'function') {
      that._dataChannels[internalDC.label] = internalDC;
      var dc = new RTCDataChannel(internalDC);
      that.ondatachannel.apply(that, [dc]);
    }
  };

  this._pc.onaddstream = function onaddstream(internalMS) {
    if(that.onaddstream && typeof that.onaddstream == 'function') {
      var ms = new RTCMediaStream(internalMS);
      that.onaddstream.apply(that, [ms]);
    }
  };

  this._pc.onremovestream = function onremovestream(internalMS) {
    if(that.onremovestream && typeof that.onremovestream == 'function') {
      var ms = new RTCMediaStream(internalMS);
      that.onremovestream.apply(that, [ms]);
    }
  };

  this.onicecandidate = null;
  this.onsignalingstatechange = null;
  this.onicegatheringstatechange = null;
  this.ondatachannel = null;
  this.onaddstream = null;
  this.onremovestream = null;
};

PeerConnection.prototype.RTCSignalingStates = [
  'stable',
  'have-local-offer',
  'have-local-pranswer',
  'have-remote-offer',
  'have-remote-pranswer',
  'closed'
];

PeerConnection.prototype.RTCIceConnectionStates = [
  'new',
  'gathering',
  'waiting',
  'checking',
  'connected',
  'failed',
  // 'disconnected',
  'closed'
];

PeerConnection.prototype.RTCIceGatheringStates = [
  'new',
  'gathering',
  'complete'
];

PeerConnection.prototype._getPC = function _getPC() {
  if(!this._pc) {
    throw new Error('RTCPeerConnection is gone');
  }
  return this._pc;
};

PeerConnection.prototype._checkClosed = function _checkClosed() {
  if(this._closed) {
    throw new Error('Peer is closed');
  }
};

PeerConnection.prototype._runImmediately = function _runImmediately(obj) {
  var pc = this._getPC();
  this._checkClosed();
  return pc[obj.func].apply(pc, obj.args);
}

PeerConnection.prototype._queueOrRun = function _queueOrRun(obj) {
  var pc = this._getPC();
  this._checkClosed();
  if(null == this._pending) {
    pc[obj.func].apply(pc, obj.args);
    if(obj.wait) {
      this._pending = obj;
    }
  } else {
    this._queue.push(obj);
  }
};

PeerConnection.prototype._executeNext = function _executeNext() {
  var obj, pc;
  pc = this._getPC();
  if(this._queue.length > 0) {
    obj = this._queue.shift();
    pc[obj.func].apply(pc, obj.args);
    if(obj.wait)
    {
      this._pending = obj;
    } else {
      this._executeNext();
    }
  } else {
    this._pending = null;
  }
};

PeerConnection.prototype.getLocalDescription = function getLocalDescription() {
  var sdp = this._getPC().localDescription;
  if(!sdp) {
    return null;
  }
  return new RTCSessionDescription({ type: this._localType,
                                      sdp: sdp });
};

PeerConnection.prototype.getRemoteDescription = function getRemoteDescription() {
  var sdp = this._getPC().remoteDescription;
  if(!sdp) {
    return null;
  }
  return new RTCSessionDescription({ type: this._remoteType,
                                      sdp: sdp });
};

PeerConnection.prototype.getSignalingState = function getSignalingState() {
  var state = this._getPC().signalingState;
  return this.RTCSignalingStates[state];
};

PeerConnection.prototype.getIceGatheringState = function getIceGatheringState() {
  var state = this._getPC().iceGatheringState;
  return this.RTCIceGatheringStates[state];
};

PeerConnection.prototype.getIceConnectionState = function getIceConnectionState() {
  var state = this._getPC().iceConnectionState;
  return this.RTCIceConnectionStates[state];
};

PeerConnection.prototype.getLocalStreams = function getLocalStreams() {
  return this._getPC().getLocalStreams();
};

PeerConnection.prototype.getRemoteStreams = function getRemoteStreams() {
  return this._getPC().getRemoteStreams();
};

PeerConnection.prototype.getStreamById = function getStreamById(id) {
  return this._getPC().getStreamById(id);
};

PeerConnection.prototype.getOnSignalingStateChange = function() {
  return this.onsignalingstatechange;
};

PeerConnection.prototype.setOnSignalingStateChange = function(cb) {
  // FIXME: throw an exception if cb isn't callable
  this.onsignalingstatechange = cb;
};

PeerConnection.prototype.getOnIceCandidate = function() {
  return this.onicecandidate;
};

PeerConnection.prototype.setOnIceCandidate = function(cb) {
  // FIXME: throw an exception if cb isn't callable
  this.onicecandidate = cb;
};

PeerConnection.prototype.getOnDataChannel = function() {
  return this.ondatachannel;
};

PeerConnection.prototype.setOnDataChannel = function(cb) {
  // FIXME: throw an exception if cb isn't callable
  this.ondatachannel = cb;
};

PeerConnection.prototype.getOnAddStream = function() {
  return this.onaddstream;
};

PeerConnection.prototype.setOnAddStream = function(cb) {
  // FIXME: throw an exception if cb isn't callable
  this.onaddstream = cb;
};

PeerConnection.prototype.getOnRemoveStream = function() {
  return this.onremovestream;
};

PeerConnection.prototype.setOnRemoveStream = function(cb) {
  // FIXME: throw an exception if cb isn't callable
  this.onremovestream = cb;
};

PeerConnection.prototype.createOffer = function createOffer(onSuccess, onError, constraints) {
  constraints = constraints || {};
  // FIXME: complain if onError is undefined
  this._queueOrRun({
    func: 'createOffer',
    args: [constraints],
    wait: true,
    onSuccess: function(sdp) {
      onSuccess.call(this, new RTCSessionDescription({type: 'offer', sdp: sdp}));
    },
    onError: onError
  });
};

PeerConnection.prototype.createAnswer = function createAnswer(onSuccess, onError, constraints) {
  constraints = constraints || {};
  // FIXME: complain if onError is undefined
  this._queueOrRun({
    func: 'createAnswer',
    args: [constraints],
    wait: true,
    onSuccess: function(sdp) {
      onSuccess.call(this, new RTCSessionDescription({type: 'answer', sdp: sdp}));
    },
    onError: onError
  });
};

PeerConnection.prototype.setLocalDescription = function setLocalDescription(sdp, onSuccess, onError) {
  sdp = sdp || {};
  this._localType = sdp.type;
  this._queueOrRun({
    func: 'setLocalDescription',
    args: [sdp],
    wait: true,
    onSuccess: onSuccess,
    onError: onError
  });
};

PeerConnection.prototype.setRemoteDescription = function setRemoteDescription(sdp, onSuccess, onError) {
  sdp = sdp || {};
  this._remoteType = sdp.type;
  this._queueOrRun({
    func: 'setRemoteDescription',
    args: [sdp],
    wait: true,
    onSuccess: onSuccess,
    onError: onError
  });
};

PeerConnection.prototype.addIceCandidate = function addIceCandidate(sdp, onSuccess, onError) {
  sdp = sdp || {};
  this._queueOrRun({
    func: 'addIceCandidate',
    args: [{'candidate': sdp.candidate, 'sdpMid': sdp.sdpMid, 'sdpMLineIndex': sdp.sdpMLineIndex}],
    wait: true,
    onSuccess: onSuccess,
    onError: onError
  });
};

PeerConnection.prototype.createDataChannel = function createDataChannel(label, dataChannelDict) {
  dataChannelDict = dataChannelDict || {};
  var channel = this._runImmediately({
    func: 'createDataChannel',
    args: [label, dataChannelDict]
  });
  this._dataChannels[label] = channel;
  return new RTCDataChannel(channel);
};

PeerConnection.prototype.addStream = function addStream(stream, constraintsDict) {
  constraintsDict = constraintsDict || {};
  return this._runImmediately({
    func: 'addStream',
    args: [stream, constraintsDict]
  });
};

PeerConnection.prototype.removeStream = function removeStream(stream) {
  return this._runImmediately({
    func: 'removeStream',
    args: [stream]
  });
};
PeerConnection.prototype.close = function close() {
  return this._runImmediately({
    func: 'close',
    args: []
  });
}

function RTCPeerConnection(configuration, constraints) {
  var pc = new PeerConnection(configuration, constraints);

  Object.defineProperties(this, {
    'localDescription': {
      get: function getLocalDescription() {
        return pc.getLocalDescription();
      }
    },
    'remoteDescription': {
      get: function getRemoteDescription() {
        return pc.getRemoteDescription();
      }
    },
    'signalingState': {
      get: function getSignalingState() {
        return pc.getSignalingState();
      }
    },
    'readyState': {
      get: function getReadyState() {
        return pc.getReadyState();
      }
    },
    'iceGatheringState': {
      get: function getIceGatheringState() {
        return pc.getIceGatheringState();
      }
    },
    'iceConnectionState': {
      get: function getIceConnectionState() {
        return pc.getIceConnectionState();
      }
    },
    'onsignalingstatechange': {
      get: function() {
        return pc.getOnSignalingStateChange();
      },
      set: function(cb) {
        pc.setOnSignalingStateChange(cb);
      }
    },
    'onicecandidate': {
      get: function() {
        return pc.getOnIceCandidate();
      },
      set: function(cb) {
        pc.setOnIceCandidate(cb);
      }
    },
    'ondatachannel': {
      get: function() {
        return pc.getOnDataChannel();
      },
      set: function(cb) {
        pc.setOnDataChannel(cb);
      }
    },
    'onaddstream': {
      get: function() {
        return pc.getOnAddStream();
      },
      set: function(cb) {
        pc.setOnAddStream(cb);
      }
    },
    'onremovestream': {
      get: function() {
        return pc.getOnRemoveStream();
      },
      set: function(cb) {
        pc.setOnRemoveStream(cb);
      }
    }
  });

  this.createOffer = function createOffer() {
    return pc.createOffer.apply(pc, arguments);
  };

  this.createAnswer = function createAnswer() {
    return pc.createAnswer.apply(pc, arguments);
  };

  this.setLocalDescription = function setLocalDescription() {
    return pc.setLocalDescription.apply(pc, arguments);
  };

  this.setRemoteDescription = function setRemoteDescription() {
    return pc.setRemoteDescription.apply(pc, arguments);
  };

  this.addIceCandidate = function addIceCandidate() {
    return pc.addIceCandidate.apply(pc, arguments);
  };

  this.createDataChannel = function createDataChannel() {
    return pc.createDataChannel.apply(pc, arguments);
  };

  this.addStream = function addStream() {
    pc.addStream.apply(pc, arguments);
  };

  this.removeStream = function removeStream() {
    pc.removeStream.apply(pc, arguments);
  };

  this.getLocalStreams = function getLocalStreams() {
    return pc.getLocalStreams.apply(pc, arguments);
  };

  this.getRemoteStreams = function getRemoteStreams() {
    return pc.getRemoteStreams.apply(pc, arguments);
  };

  this.getStreamById = function getStreamById() {
    return pc.getStreamById.apply(pc, arguments);
  };
  this.close = function close() {
    return pc.close.apply(pc, arguments);
  }
};

module.exports = RTCPeerConnection;