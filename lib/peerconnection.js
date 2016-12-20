var path = require('path');
var binary = require('node-pre-gyp');
var path = require('path');
//var binding_opts = require(path.resolve(path.join(__dirname, '../build/wrtc.json')));
var binding_path = binary.find(path.resolve(path.join(__dirname,'../package.json')));
var _webrtc = require(binding_path);

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
    , localType = null
    , remoteType = null
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

  pc.onsignalingstatechange = function onsignalingstatechange(state) {
    var stateString = that.RTCSignalingStates[state];

    if('closed' === stateString) {
      Object.keys(dataChannels).forEach(function(label) {
        dataChannels[label].shutdown();

        delete dataChannels[label];
      });
    }

    that.dispatchEvent({type: 'signalingstatechange'});
  };

  pc.oniceconnectionstatechange = function oniceconnectionstatechange(state) {
    that.dispatchEvent({type: 'iceconnectionstatechange'});
  };

  pc.onicegatheringstatechange = function onicegatheringstatechange(state) {
    that.dispatchEvent({type: 'icegatheringstatechange'});

    // if we have completed gathering candidates, trigger a null candidate event
    if (that.RTCIceGatheringStates[state] === 'complete') {
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
        var sdp = pc.localDescription;
        if(!sdp) {
          return null;
        }
        return new RTCSessionDescription({ type: localType, sdp: sdp });
      }
    },
    'remoteDescription': {
      get: function getRemoteDescription() {
        var sdp = pc.remoteDescription;
        if(!sdp) {
          return null;
        }
        return new RTCSessionDescription({ type: remoteType, sdp: sdp });
      }
    },
    'signalingState': {
      get: function getSignalingState() {
        var state = pc.signalingState;
        return this.RTCSignalingStates[state];
      }
    },
    'readyState': {
      get: function getReadyState() {
        return pc.getReadyState();
      }
    },
    'iceGatheringState': {
      get: function getIceGatheringState() {
        var state = pc.iceGatheringState;
        return this.RTCIceGatheringStates[state];
      }
    },
    'iceConnectionState': {
      get: function getIceConnectionState() {
        var state = pc.iceConnectionState;
        return this.RTCIceConnectionStates[state];
      }
    }
  });

  this.createOffer = function createOffer(){
    var call = function call(options, successCallback, failureCallback) {
      queueOrRun({
        func: 'createOffer',
        args: [options],
        wait: true,
        onSuccess: function(sdp) {
          successCallback.call(this, new RTCSessionDescription({type: 'offer', sdp: sdp}));
        },
        onError: failureCallback
      });
    };

    if (arguments.length === 0 || arguments.length === 1 && typeof arguments[0] === 'object') {      
      // Promise-based call.
      var options = (arguments.length === 1) ? arguments[0] : {};
      return new Promise(function createOfferPromise(resolve, reject) {
        call(options, resolve, reject);
      });
    } else if (arguments.length >= 2
        && typeof arguments[0] === 'function'
        && typeof arguments[1] === 'function'
        && (arguments.length === 2 || typeof arguments[2] === 'object' || typeof arguments[2] === 'undefined')) {
      // Legacy method.
      var options = (arguments.length === 3 && typeof arguments[2] === 'object') ? arguments[2] : {};
      call(options, arguments[0], arguments[1]);
    } else {
      throw new Error('Invalid call to createOffer - function must either have prototype'
        + ' ([config]) or (successCallback, failureCallback, [config]).');
    }
  };

  this.createAnswer = function createAnswer(){
    var call = function call(options, successCallback, failureCallback) {
      queueOrRun({
        func: 'createAnswer',
        args: [options],
        wait: true,
        onSuccess: function(sdp) {
          successCallback.call(this, new RTCSessionDescription({type: 'answer', sdp: sdp}));
        },
        onError: failureCallback
      });
    };

    if (arguments.length === 0 || arguments.length === 1 && typeof arguments[0] === 'object') {      
      // Promise-based call.
      var options = (arguments.length === 1) ? arguments[0] : {};
      return new Promise(function createAnswerPromise(resolve, reject) {
        call(options, resolve, reject);
      });
    } else if (arguments.length >= 2
        && typeof arguments[0] === 'function'
        && typeof arguments[1] === 'function') {
      // Legacy method.
      call({}, arguments[0], arguments[1]);
    } else {
      throw new Error('Invalid call to createAnswer - function must either have prototype'
        + ' (void) or (successCallback, failureCallback).');
    }
  };

  this.setLocalDescription = function setLocalDescription(){
    var call = function call(description, successCallback, failureCallback) {
      localType = description.type;

      queueOrRun({
        func: 'setLocalDescription',
        args: [description],
        wait: true,
        onSuccess: successCallback,
        onError: failureCallback
      });
    };

    var error = new Error('Invalid call to setLocalDescription - function must either have prototype'
        + ' (description) or (description, successCallback, failureCallback).');

    if (arguments.length >= 1 && typeof arguments[0] === 'object') {      
      var description = arguments[0];

      if(arguments.length === 1) {
        // Promise-based call.
        return new Promise(function setLocalDescriptionPromise(resolve, reject) {
          call(description, resolve, reject);
        });
      } else if (arguments.length >= 3
          && typeof arguments[1] === 'function'
          && typeof arguments[2] === 'function') {
        // Legacy method.
        call(description, arguments[1], arguments[2]);
      } else {
        throw error;
      }

    } else {
      throw error;
    }
  };

  this.setRemoteDescription = function setRemoteDescription(){
    var call = function call(description, successCallback, failureCallback) {
      remoteType = description.type;

      queueOrRun({
        func: 'setRemoteDescription',
        args: [description],
        wait: true,
        onSuccess: successCallback,
        onError: failureCallback
      });
    };

    var error = new Error('Invalid call to setRemoteDescription - function must either have prototype'
        + ' (description) or (description, successCallback, failureCallback).');

    if (arguments.length >= 1 && typeof arguments[0] === 'object') {      
      var description = arguments[0];

      if(arguments.length === 1) {
        // Promise-based call.
        return new Promise(function setRemoteDescriptionPromise(resolve, reject) {
          call(description, resolve, reject);
        });
      } else if (arguments.length >= 3
          && typeof arguments[1] === 'function'
          && typeof arguments[2] === 'function') {
        // Legacy method.
        call(description, arguments[1], arguments[2]);
      } else {
        throw error;
      }

    } else {
      throw error;
    }
  };

  this.addIceCandidate = function addIceCandidate(){
    var call = function call(candidate, successCallback, failureCallback) {
      queueOrRun({
        func: 'addIceCandidate',
        args: [{'candidate':     candidate.candidate,
                'sdpMid':        candidate.sdpMid,
                'sdpMLineIndex': candidate.sdpMLineIndex
               }],
        wait: true,
        onSuccess: successCallback,
        onError: failureCallback
      });
    };

    var error = new Error('Invalid call to addIceCandidate - function must either have prototype'
        + ' (candidate) or (candidate, successCallback, failureCallback).');

    if (arguments.length >= 1 && typeof arguments[0] === 'object') {      
      var candidate = arguments[0];

      if(arguments.length === 1) {
        // Promise-based call.
        return new Promise(function addIceCandidatePromise(resolve, reject) {
          call(candidate, resolve, reject);
        });
      } else if (arguments.length >= 3
          && typeof arguments[1] === 'function'
          && typeof arguments[2] === 'function') {
        // Legacy method.
        call(candidate, arguments[1], arguments[2]);
      } else {
        throw error;
      }

    } else {
      throw error;
    }
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

RTCPeerConnection.prototype.RTCIceConnectionStates = [
  'new',
  'checking',
  'connected',
  'completed',
  'failed',
  'disconnected',
  'closed'
];

RTCPeerConnection.prototype.RTCIceGatheringStates = [
  'new',
  'gathering',
  'complete'
];

RTCPeerConnection.prototype.RTCSignalingStates = [
  'stable',
  'have-local-offer',
  'have-local-pranswer',
  'have-remote-offer',
  'have-remote-pranswer',
  'closed'
];


module.exports = RTCPeerConnection;
