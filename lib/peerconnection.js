'use strict';

var _webrtc = require('./binding');

var EventTarget = require('./eventtarget');

var RTCDataChannel = require('./datachannel');
var RTCDataChannelEvent = require('./datachannelevent');
var RTCMediaStreamEvent       = require('./mediastreamevent');
var RTCIceCandidate = require('./icecandidate');
var RTCPeerConnectionIceEvent = require('./rtcpeerconnectioniceevent');
var RTCSessionDescription = require('./sessiondescription');
var RTCStatsResponse = require('./rtcstatsresponse');
var RTCMediaStream            = require('./mediastream');
var RTCVideoFrameEvent        = require('./rtcvideoframeevent');

function RTCPeerConnection(configuration, constraints) {
  var self = this;
  var pc = new _webrtc.PeerConnection(configuration, constraints);
  var localType = null;
  var remoteType = null;
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

  function runImmediately(obj) {
    return pc[obj.func].apply(pc, obj.args);
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

  pc.onicecandidate = function onicecandidate(candidate, sdpMid, sdpMLineIndex) {
    var icecandidate = new RTCIceCandidate({
      candidate: candidate,
      sdpMid: sdpMid,
      sdpMLineIndex: sdpMLineIndex
    });

    var iceCandidateEvent = new RTCPeerConnectionIceEvent('icecandidate', {candidate: icecandidate});
    self.dispatchEvent(iceCandidateEvent);
    self._onicecandidate && self._onicecandidate(iceCandidateEvent);
  };

  pc.onsignalingstatechange = function onsignalingstatechange(state) {
    self.dispatchEvent({type: 'signalingstatechange'});
    self._onsignalingstatechange && self._onsignalingstatechange({type: 'signalingstatechange'});
  };

  pc.oniceconnectionstatechange = function oniceconnectionstatechange(state) {
    self.dispatchEvent({type: 'iceconnectionstatechange'});
    self._oniceconnectionstatechange && self._oniceconnectionstatechange({type: 'iceconnectionstatechange'});
  };

  pc.onicegatheringstatechange = function onicegatheringstatechange(state) {
    self.dispatchEvent({type: 'icegatheringstatechange'});

    self._onicegatheringstatechange && self._onicegatheringstatechange({type: 'icegatheringstatechange'});
    // if we have completed gathering candidates, trigger a null candidate event
    if (self.RTCIceGatheringStates[state] === 'complete') {
      var iceCandidateEvent = new RTCPeerConnectionIceEvent('icecandidate', {candidate: null});
      self.dispatchEvent(iceCandidateEvent);
      self._onicecandidate && self._onicecandidate(iceCandidateEvent);
    }
  };
  pc.ondatachannel = function ondatachannel(internalDC) {
    var dc = new RTCDataChannel(internalDC);

      var dcEvent = new RTCDataChannelEvent('datachannel', {channel: dc});
      self.dispatchEvent(dcEvent);
      self._ondatachannelcb && self._ondatachannelcb(dcEvent);
  }
  pc.onaddstream = function(internalMS) {
    var ms = new RTCMediaStream(internalMS);

    var addStreamEvent = new RTCMediaStreamEvent('mediastream', {stream: ms})
    self.dispatchEvent(addStreamEvent);
    self._onaddstream && self._onaddstream(addStreamEvent);

  };

  pc.onvideoframe = function(id, width, height, yPlane, uPlane, vPlane) {
    var videoFrameEvent = new RTCVideoFrameEvent('videoframe',
        {id: id, width: width, height: height, yPlane: yPlane, uPlane: uPlane, vPlane: vPlane});
    self.dispatchEvent(videoFrameEvent);
    self._onvideoframe && self._onvideoframe(videoFrameEvent);
  };

  pc.onencodedvideoframe = function(id, width, height, encodedFrame, frameType, timestamp) {
      var videoFrameEvent = new RTCVideoFrameEvent('videoframe',
          {id: id, width: width, height: height, encodedFrame: encodedFrame,
              frameType: frameType, timestamp: timestamp});
      self.dispatchEvent(videoFrameEvent);
      self._onencodedvideoframe && self._onencodedvideoframe(videoFrameEvent);
  };

  pc.onnegotiationneeded = function() {
    self.dispatchEvent({type: 'negotiationneeded'});
    self._onnegotiationneeded && self._onnegotiationneeded();
  }

  //
  // PeerConnection properties & attributes
  //

  Object.defineProperties(this, {
    ondatachannel: {
      set: function(onDC) {
        self._ondatachannelcb = onDC;
      }
    },
    onaddstream: {
      set: function(onAddStream) {
        self._onaddstream = onAddStream;
      }
    },
    onicegatheringstatechange : {
      set: function(onIceGatheringStateChange) {
        self._onicegatheringstatechange = onIceGatheringStateChange;
      }
    },
    oniceconnectionstatechange: {
      set: function(onIceConnectionStateChange) {
        self._oniceconnectionstatechange = onIceConnectionStateChange;
      }
    },
    onsignalingstatechange: {
      set: function(onSignalingStateChange) {
        self._onsignalingstatechange = onSignalingStateChange;
      }
    },
    onicecandidate: {
      set: function(onIceCandidate) {
        self._onicecandidate = onIceCandidate;
      }
    },
    onvideoframe: {
      set: function(onVideoFrame) {
        self._onvideoframe = onVideoFrame;
      }
    },
    onencodedvideoframe: {
      set: function(onEncodedVideoFrame) {
          self._onencodedvideoframe = onEncodedVideoFrame;
      }
    },
    onnegotiationneeded: {
      set: function(onNegotiationNeeded) {
        self._onnegotiationneeded = onNegotiationNeeded;
      }
    },
    localDescription: {
      get: function getLocalDescription() {
        var sdp = pc.localDescription;
        if (!sdp) {
          return null;
        }
        return new RTCSessionDescription({ type: localType, sdp: sdp });
      }
    },
    remoteDescription: {
      get: function getRemoteDescription() {
        var sdp = pc.remoteDescription;
        if (!sdp) {
          return null;
        }
        return new RTCSessionDescription({ type: remoteType, sdp: sdp });
      }
    },
    signalingState: {
      get: function getSignalingState() {
        var state = pc.signalingState;
        return this.RTCSignalingStates[state];
      }
    },
    readyState: {
      get: function getReadyState() {
        return pc.getReadyState();
      }
    },
    iceGatheringState: {
      get: function getIceGatheringState() {
        var state = pc.iceGatheringState;
        return this.RTCIceGatheringStates[state];
      }
    },
    iceConnectionState: {
      get: function getIceConnectionState() {
        var state = pc.iceConnectionState;
        return this.RTCIceConnectionStates[state];
      }
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
      localType = description.type;

      queueOrRun({
        func: 'setLocalDescription',
        args: [description],
        wait: true,
        onSuccess: successCallback,
        onError: failureCallback
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
      remoteType = description.type;

      queueOrRun({
        func: 'setRemoteDescription',
        args: [description],
        wait: true,
        onSuccess: successCallback,
        onError: failureCallback
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

  this.addStream = function addStream() {
      var call = function call(stream, successCallback, failureCallback) {
          queueOrRun({
              func: 'addStream',
              args: [stream],
              wait: true,
              onSuccess: successCallback,
              onError: failureCallback
          });
      }
      var error = new Error('Invalid call to addStream - function must either have prototype (stream ) or (stream, successCallback, failureCallback).');
      if (arguments.length >= 1 && typeof arguments[0] === 'object') {
          var stream = arguments[0]._internalMS;
          if (arguments.length === 1) {
              // Promise-based call
              return new Promise(function addStreamPromise(resolve, reject) {
                  call(stream, resolve, reject);
              });
          }
          else if (arguments.length >= 3 &&
              typeof(arguments[1]) === 'function' &&
              typeof(arguments[2]) === 'function') {
              // Legacy method.
              call(stream, arguments[1], arguments[2]);
          }
          else {
              throw error;
          }
      }
  }

   // eslint-disable-next-line consistent-return
  this.createDataChannel = function createDataChannel(label, dataChannelDict) {
    dataChannelDict = dataChannelDict || {};

    var channel = runImmediately({
      func: 'createDataChannel',
      args: [label, dataChannelDict]
    });

    // channel will be undefined if dataChannel was closed before calling this function
    // (see peerconnection.cc)
    if (channel) {
        return new RTCDataChannel(channel);
    }
  };

  this.getStats = function getStats(onSuccess, onFailure) {
    pc.getStats(function(internalRTCStatsResponse) {
      onSuccess(new RTCStatsResponse(internalRTCStatsResponse));
    }, onFailure);
  };

  this.onStreamVideoFrame = function(stream, callback) {
    var videoTrackIds = stream.getVideoTracks().map(function(vTracks) {
        return vTracks.id;
    });

    self.addEventListener('videoframe', function(frame) {
      if(videoTrackIds.find(function(tId) {
        return tId === frame.id
      })) {
        callback(frame);
        return;
      }
    });

    pc.onStreamVideoFrame(stream._internalMS, callback);
  };

  this.onStreamEncodedVideoFrame = function(stream, callback) {
      var videoTrackIds = stream.getVideoTracks().map(function(vTracks) {
          return vTracks.id;
      });

      // Callback if video track is for this stream.
      callback && self.addEventListener('videoframe', function(frame) {
          if(videoTrackIds.find(function(tId) {
                  return tId === frame.id
              })) {
              callback(frame);
              return;
          }
      });

      pc.onStreamEncodedVideoFrame(stream._internalMS, callback);
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
