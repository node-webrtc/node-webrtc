var _webrtc = require('bindings')('webrtc.node');

function RTCIceCandidate(dict) {
	this.candidate = dict.candidate;
	this.sdpMid = dict.sdpMid;
	this.sdpMLineIndex = dict.hasOwnProperty('sdpMLineIndex') ? dict.sdpMLineIndex : 0;
};

function RTCSessionDescription(dict) {
	this.type = dict.type;
	this.sdp = dict.sdp;
};

function RTCError(code, message) {
	this.name = this.reasonName[Math.min(code, this.reasonName.length - 1)];
	this.message = (typeof message === "string")? message : this.name;
}

RTCError.prototype.reasonName = [
	// These strings must match those defined in the WebRTC spec.
	"NO_ERROR", // Should never happen -- only used for testing
  "INVALID_CONSTRAINTS_TYPE",
  "INVALID_CANDIDATE_TYPE",
  "INVALID_MEDIASTREAM_TRACK",
  "INVALID_STATE",
  "INVALID_SESSION_DESCRIPTION",
  "INCOMPATIBLE_SESSION_DESCRIPTION",
  "INCOMPATIBLE_CONSTRAINTS",
  "INCOMPATIBLE_MEDIASTREAMTRACK",
  "INTERNAL_ERROR"
];


function DataChannel(_dc) {
	var that = this;
	this._dc = _dc;

	this._queue = [];
	this._pending = null;

	this._dc.onerror = function onerror() {

	};

	this._dc.onmessage = function onmessage() {

	};

	this._dc.onopen = function onopen() {

	};

	this._dc.onclose = function onclose() {

	};

	this.onerror = null;
	this.onmessage = null;
	this.onopen = null;
	this.onclose = null;
};

DataChannel.prototype._getDC = function _getDC() {
	if(!this._pc) {
		throw new Error('RTCDataChannel is gone');
	}
	return this._dc;
};

DataChannel.prototype._queueOrRun = function _queueOrRun(obj) {
	var pc = this._getPC();
	// this._checkClosed();
	if(null == this._pending) {
		pc[obj.func].apply(pc, obj.args);
		if(obj.wait) {
			this._pending = obj;
		}
	} else {
		this._queue.push(obj);
	}
};

DataChannel.prototype._executeNext = function _executeNext() {
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


function PeerConnection(configuration, constraints) {
	var that = this;
	this._pc = new _webrtc.PeerConnection(configuration, constraints);

	this._localType = null;
	this._remoteType = null;

	this.iceGatheringState = '';
	this.iceConnectionState = '';
	this.signalingState = '';
	this.readyState = '';

	this._queue = [];
	this._pending = null;

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

	this._pc.onstatechange = function onstatechange(type, state) {
		var typeString = that.RTCStateTypes[type];
		var stateString = null;
		var callback = null;
		switch(typeString)
		{
			case 'ready':
				stateString = that.readyState = that.RTCReadyStates[state];
				break;
			case 'ice':
				stateString = that.iceConnectionState = that.RTCIceConnectionStates[state];
				callback = that.oniceconnectionstatechange;
				break;
			case 'sdp':
				// don't care
				break;
			case 'sipcc':
				// don't care
				break;
			case 'signaling':
				stateString = that.signalingState = that.RTCSignalingStates[state];
				callback = that.onsignalingstatechange;
				break;
		}
		if(callback && typeof callback == 'function') {
			callback.apply(that, [stateString]);
		}
	};

	this._pc.ondatachannel = function ondatachannel(dataChannel) {
		if(that.ondatachannel && typeof that.ondatachannel == 'function') {
			var dc = new DataChannel(dataChannel);
			that.ondatachannel.apply(that, [dc]);
		}
	};

	this.onicecandidate = null;
	this.onsignalingstatechange = null;
	this.ondatachannel = null;
};

PeerConnection.prototype.RTCStateTypes = [
	undefined,
	'ready',
	'ice',
	'sdp',
	'sipcc',
	'signaling'
];

PeerConnection.prototype.RTCReadyStates = [
	'new',
	'negotiating',
	'active',
	'closing',
	'closed'
];

PeerConnection.prototype.RTCSignalingStates = [
	'stable',
	'have-local-offer',
	'have-local-pranswer',
	'have-remote-offer',
	'have-remote-pranswer',
	'closed'
];

PeerConnection.prototype.RTCIceConnectionStates = [
	'gathering',
	'waiting',
	'checking',
	'connected',
	'failed',
	'disconnected',
	'closed'
];

PeerConnection.prototype.RTCSipccStates = [
	'idle',
	'starting',
	'started'
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
	if(0 == sdp.length) {
		return null;
	}
	return new RTCSessionDescription({ type: this._localType,
	                                    sdp: sdp });
};

PeerConnection.prototype.getRemoteDescription = function getRemoteDescription() {
	var sdp = this._getPC().remoteDescription;
	if(0 == sdp.length) {
		return null;
	}
	return new RTCSessionDescription({ type: this._remoteType,
	                                    sdp: sdp });
};

PeerConnection.prototype.getSignalingState = function getSignalingState() {
	if(this._closed) {
		return 'closed';
	}
	return this.signalingState;
};

PeerConnection.prototype.getIceGatheringState = function getIceGatheringState() {
	return this.iceGatheringState;
};

PeerConnection.prototype.getIceConnectionState = function getIceConnectionState() {
	return this.iceConnectionState;
};

PeerConnection.prototype.getReadyState = function getReadyState() {
	return this.readyState;
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
		}
	});

	this.createOffer = function createOffer() {
		pc.createOffer.apply(pc, arguments);
	};

	this.createAnswer = function createAnswer() {
		pc.createAnswer.apply(pc, arguments);
	};

	this.setLocalDescription = function setLocalDescription() {
		pc.setLocalDescription.apply(pc, arguments);
	};

	this.setRemoteDescription = function setRemoteDescription() {
		pc.setRemoteDescription.apply(pc, arguments);
	};

	this.addIceCandidate = function addIceCandidate() {
		pc.addIceCandidate.apply(pc, arguments);
	};
};

exports.RTCPeerConnection = RTCPeerConnection;
exports.RTCSessionDescription = RTCSessionDescription;
exports.RTCIceCandidate = RTCIceCandidate;
