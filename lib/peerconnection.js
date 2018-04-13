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
  var queue = [];
  var pending = null;

  EventTarget.call(this);

  function executeNext() {
    if (queue.length > 0) {
      var obj = queue.shift();

      pc[obj.func].apply(pc, obj.args);

      if (obj.wait)
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
    if (null === pending) {
      pc[obj.func].apply(pc, obj.args);

      if (obj.wait) {
        pending = obj;
      }
    } else {
      queue.push(obj);
    }
  }

  //
  // Attach events to the native PeerConnection object
  //

  pc.onerror = function onerror() {
    if (pending && pending.onError) {
      pending.onError.apply(self, arguments);
    }

    executeNext();
  };

  pc.onsuccess = function onsuccess() {
    if (pending && pending.onSuccess) {
      pending.onSuccess.apply(self, arguments);
    }

    executeNext();
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

  // eslint-disable-next-line consistent-return
  this.createOffer = function createOffer() {
    function call(options, successCallback, failureCallback) {
      queueOrRun({
        func: 'createOffer',
        args: [options],
        wait: true,
        onSuccess: function(sdp) {
          successCallback.call(this, new RTCSessionDescription({ type: 'offer', sdp: sdp }));
        },
        onError: failureCallback
      });
    }

    var options;
    if (arguments.length === 0 || arguments.length === 1 && typeof arguments[0] === 'object') {
      // Promise-based call.
      options = (arguments.length === 1) ? arguments[0] : {};
      return new Promise(function createOfferPromise(resolve, reject) {
        call(options, resolve, reject);
      });
    } else if (arguments.length >= 2
        && typeof arguments[0] === 'function'
        && typeof arguments[1] === 'function'
        && (arguments.length === 2 || typeof arguments[2] === 'object' || typeof arguments[2] === 'undefined')) {
      // Legacy method.
      options = (arguments.length === 3 && typeof arguments[2] === 'object') ? arguments[2] : {};
      call(options, arguments[0], arguments[1]);
    } else {
      throw new Error('Invalid call to createOffer - function must either have prototype'
        + ' ([config]) or (successCallback, failureCallback, [config]).');
    }
  };

  // eslint-disable-next-line consistent-return
  this.createAnswer = function createAnswer() {
    function call(options, successCallback, failureCallback) {
      queueOrRun({
        func: 'createAnswer',
        args: [options],
        wait: true,
        onSuccess: function(sdp) {
          successCallback.call(this, new RTCSessionDescription({ type: 'answer', sdp: sdp }));
        },
        onError: failureCallback
      });
    }

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

  // eslint-disable-next-line consistent-return
  this.setLocalDescription = function setLocalDescription() {
    function call(description, successCallback, failureCallback) {
      queueOrRun({
        func: 'setLocalDescription',
        args: [description],
        wait: true,
        onSuccess: successCallback,
        onError: function(error) {
          // TODO(mroberts): Start using the domexception library.
          if (error.message.match(/Called in wrong state:/)) {
            error.code = 11;
            error.name = 'InvalidStateError';
          } else if (error.message.match(/does not match/)) {
            error.code = 13;
            error.name = 'InvalidModificationError';
          }
          failureCallback(error);
        }
      });
    }

    var error = new Error('Invalid call to setLocalDescription - function must either have prototype'
        + ' (description) or (description, successCallback, failureCallback).');

    if (arguments.length >= 1 && typeof arguments[0] === 'object') {
      var description = arguments[0];

      if (arguments.length === 1) {
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

  // eslint-disable-next-line consistent-return
  this.setRemoteDescription = function setRemoteDescription() {
    function call(description, successCallback, failureCallback) {
      queueOrRun({
        func: 'setRemoteDescription',
        args: [description],
        wait: true,
        onSuccess: successCallback,
        onError: function(error) {
          // TODO(mroberts): Start using the domexception library.
          if (error.message.match(/Called in wrong state:/)) {
            error.code = 11;
            error.name = 'InvalidStateError';
          }
          failureCallback(error);
        }
      });
    }

    var error = new Error('Invalid call to setRemoteDescription - function must either have prototype'
        + ' (description) or (description, successCallback, failureCallback).');

    if (arguments.length >= 1 && typeof arguments[0] === 'object') {
      var description = arguments[0];

      if (arguments.length === 1) {
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

  // eslint-disable-next-line consistent-return
  this.addIceCandidate = function addIceCandidate() {
    function call(candidate, successCallback, failureCallback) {
      queueOrRun({
        func: 'addIceCandidate',
        args: [{ candidate: candidate.candidate,
                 sdpMid: candidate.sdpMid,
                 sdpMLineIndex: candidate.sdpMLineIndex
               }],
        wait: true,
        onSuccess: successCallback,
        onError: failureCallback
      });
    }

    var error = new Error('Invalid call to addIceCandidate - function must either have prototype'
        + ' (candidate) or (candidate, successCallback, failureCallback).');

    if (arguments.length >= 1 && typeof arguments[0] === 'object') {
      var candidate = arguments[0];

      if (arguments.length === 1) {
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
}

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

RTCPeerConnection.prototype.getConfiguration = function getConfiguration() {
  return this._pc.getConfiguration();
};

RTCPeerConnection.prototype.getStats = function getStats(onSuccess, onFailure) {
  this._pc.getStats(function(internalRTCStatsResponse) {
    onSuccess(new RTCStatsResponse(internalRTCStatsResponse));
  }, onFailure);
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

module.exports = RTCPeerConnection;
