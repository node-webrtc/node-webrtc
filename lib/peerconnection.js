var _webrtc = require('./binding');

var EventTarget = require('./eventtarget');

var RTCDataChannel            = require('./datachannel');
var RTCDataChannelEvent       = require('./datachannelevent');
var RTCError                  = require('./error');
var RTCIceCandidate           = require('./icecandidate');
var RTCPeerConnectionIceEvent = require('./rtcpeerconnectioniceevent');
var RTCSessionDescription     = require('./sessiondescription');
var RTCStatsResponse          = require('./rtcstatsresponse');

function RTCPeerConnection(configuration, constraints) {
  'use strict';
  var that = this
    , pc = new _webrtc.PeerConnection(configuration, constraints)
    , queue = []
    , pending = null
    , dataChannels = {};  // open data channels, indexed by label

  EventTarget.call(this);

  function checkClosed() {
//    if(this._closed) {
//      throw new Error('Peer is closed');
//    }
  }

  function executeNext() {
    if(queue.length > 0) {
      var obj = queue.shift();

      pc[obj.func].apply(pc, obj.args);

      if(obj.wait)
      {
        pending = obj;
      } else {
        executeNext();
      }
    } else {
      pending = null;
    }
  }

  function queueOrRun(obj) {
    checkClosed();

    if(null === pending) {
      pc[obj.func].apply(pc, obj.args);

      if(obj.wait) {
        pending = obj;
      }
    } else {
      queue.push(obj);
    }
  }

  function runImmediately(obj) {
    checkClosed();

    return pc[obj.func].apply(pc, obj.args);
  }

  //
  // Attach events to the native PeerConnection object
  //

  pc.onerror = function onerror() {
    if(pending && pending.onError) {
      pending.onError.apply(that, arguments);
    }

    executeNext();
  };

  pc.onsuccess = function onsuccess() {
    if(pending && pending.onSuccess) {
      pending.onSuccess.apply(that, arguments);
    }

    executeNext();
  };

  pc.onicecandidate = function onicecandidate(candidate, sdpMid, sdpMLineIndex) {
    var icecandidate = new RTCIceCandidate({
      candidate:     candidate,
      sdpMid:        sdpMid,
      sdpMLineIndex: sdpMLineIndex
    });

    that.dispatchEvent(new RTCPeerConnectionIceEvent('icecandidate', {candidate: icecandidate}));
  };

  pc.onsignalingstatechange = function onsignalingstatechange() {
    if (pc.signalingState === 'closed') {
      Object.keys(dataChannels).forEach(function(label) {
        dataChannels[label].shutdown();

        delete dataChannels[label];
      });
    }

    that.dispatchEvent({type: 'signalingstatechange'});
  };

  pc.oniceconnectionstatechange = function oniceconnectionstatechange() {
    that.dispatchEvent({type: 'iceconnectionstatechange'});
  };

  pc.onicegatheringstatechange = function onicegatheringstatechange() {
    that.dispatchEvent({type: 'icegatheringstatechange'});

    // if we have completed gathering candidates, trigger a null candidate event
    if (pc.iceGatheringState === 'complete') {
      that.dispatchEvent(new RTCPeerConnectionIceEvent('icecandidate', {candidate: null}));
    }
  };

  // [ToDo] onnegotiationneeded

  pc.ondatachannel = function ondatachannel(internalDC) {
    dataChannels[internalDC.label] = internalDC;

    var dc = new RTCDataChannel(internalDC);

    that.dispatchEvent(new RTCDataChannelEvent('datachannel', {channel: dc}));
  };

  //
  // PeerConnection properties & attributes
  //

  Object.defineProperties(this, {
    'localDescription': {
      get: function getLocalDescription() {
        var description = pc.localDescription;
        return description && new RTCSessionDescription(description);
      }
    },
    'remoteDescription': {
      get: function getRemoteDescription() {
        var description = pc.remoteDescription;
        return description && new RTCSessionDescription(description);
      }
    },
    'signalingState': {
      get: function getSignalingState() {
        return pc.signalingState;
      }
    },
    'readyState': {
      get: function getReadyState() {
        return pc.getReadyState();
      }
    },
    'iceGatheringState': {
      get: function getIceGatheringState() {
        return pc.iceGatheringState;
      }
    },
    'iceConnectionState': {
      get: function getIceConnectionState() {
        return pc.iceConnectionState;
      }
    }
  });

  this.createOffer = function createOffer() {
    var promise = pc.createOffer(arguments.length === 3 ? arguments[2] : arguments[0]).then(function(description) {
      return new RTCSessionDescription(description);
    });
    if (arguments.length > 1) {
      promise.then(arguments[0], arguments[1]);
    }
    return promise;
  };

  this.createAnswer = function createAnswer() {
    var promise = pc.createAnswer(arguments.length === 3 ? arguments[2] : arguments[0]).then(function(description) {
      return new RTCSessionDescription(description);
    });
    if (arguments.length > 1) {
      promise.then(arguments[0], arguments[1]);
    }
    return promise;
  };

  this.setLocalDescription = function setLocalDescription() {
    var promise = pc.setLocalDescription(arguments[0]);
    if (arguments.length === 3) {
      promise.then(arguments[1], arguments[2]);
    }
    return promise;
  };

  this.setRemoteDescription = function setRemoteDescription() {
    var promise = pc.setRemoteDescription(arguments[0]);
    if (arguments.length === 3) {
      promise.then(arguments[1], arguments[2]);
    }
    return promise;
  };

  this.addIceCandidate = function addIceCandidate() {
    var promise = pc.addIceCandidate(arguments[0]);
    if (arguments.length === 3) {
      promise.then(arguments[1], arguments[2]);
    }
    return promise;
  };

  this.createDataChannel = function createDataChannel(label, dataChannelDict) {
    dataChannelDict = dataChannelDict || {};

    var channel = runImmediately({
      func: 'createDataChannel',
      args: [label, dataChannelDict]
    });

    // channel will be undefined if dataChannel was closed before calling this function
    // (see peerconnection.cc)
    if (channel) {  
        dataChannels[label] = channel;
        return new RTCDataChannel(channel);
    }
  };

  this.getStats = function getStats(onSuccess, onFailure) {
    pc.getStats(function(internalRTCStatsResponse) {
      onSuccess(new RTCStatsResponse(internalRTCStatsResponse));
    }, onFailure);
  };

  this.close = function close() {
    return runImmediately({
      func: 'close',
      args: []
    });
  };
}

module.exports = RTCPeerConnection;
