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


function DataChannel(internalDC) {
	var that = this;
	this._dc = internalDC;

	this._queue = [];
	this._pending = null;

	this._dc.onerror = function onerror() {

	};

	this._dc.onmessage = function onmessage(data) {
		if(that.onmessage && typeof that.onmessage == 'function') {
			that.onmessage.apply(that, [data]);
		}
	};

	this._dc.onstatechange = function onstatechange() {
		var state = that.getReadyState();
		if('open' == state) {
			if(that.onopen && typeof that.onopen == 'function') {
				that.onopen.apply(that, []);
			}
		} else if('closed' == state) {
			if(that.onclose && typeof that.onclose == 'function') {
				that.onclose.apply(that, []);
			}
		}
	};

	this.onerror = null;
	this.onmessage = null;
	this.onopen = null;
	this.onclose = null;
};

DataChannel.prototype._getDC = function _getDC() {
	if(!this._dc) {
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

DataChannel.prototype.RTCDataStates = [
	'connecting',
	'open',
	'closing',
	'closed'
];

DataChannel.prototype.BinaryTypes = [
  'blob',
  'arraybuffer'
];

DataChannel.prototype.send = function send() {

};

DataChannel.prototype.close = function close() {

};

DataChannel.prototype.getLabel = function getLabel() {
	return this._getDC().label;
};

DataChannel.prototype.getReadyState = function getReadyState() {
	var state = this._getDC().readyState;
	return this.RTCDataStates[state];
};

DataChannel.prototype.getBinaryType = function getBinaryType() {
	var type = this._getDC().binaryType;
	return this.BinaryTypes[type];
};

DataChannel.prototype.setBinaryType = function setBinaryType(type) {
	var typenum = this.BinaryTypes.indexOf(type);
	if(typenum >= 0) {
		this._getDC().binaryType = typenum;
	}
};

DataChannel.prototype.getOnError = function() {
	return this.onerror;
};

DataChannel.prototype.setOnError = function(cb) {
	// FIXME: throw an exception if cb isn't callable
	this.onerror = cb;
};

DataChannel.prototype.getOnOpen = function() {
	return this.onopen;
};

DataChannel.prototype.setOnOpen = function(cb) {
	// FIXME: throw an exception if cb isn't callable
	this.onopen = cb;
};

DataChannel.prototype.getOnMessage = function() {
	return this.onmessage;
};

DataChannel.prototype.setOnMessage = function(cb) {
	// FIXME: throw an exception if cb isn't callable
	this.onmessage = cb;
};

DataChannel.prototype.getOnClose = function() {
	return this.onclose;
};

DataChannel.prototype.setOnClose = function(cb) {
	// FIXME: throw an exception if cb isn't callable
	this.onclose = cb;
};


function RTCDataChannel(internalDC) {
	var dc = new DataChannel(internalDC);

	Object.defineProperties(this, {
		'label': {
			get: function getLabel() {
				return dc.getLabel();
			}
		},
		'readyState': {
			get: function getReadyState() {
				return dc.getReadyState();
			}
		},
		'binaryType': {
			get: function getBinaryType() {
				return dc.getBinaryType();
			},
			set: function(type) {
				dc.setBinaryType(type);
			}
		},
		'onerror': {
			get: function() {
				return dc.getOnError();
			},
			set: function(cb) {
				dc.setOnError(cb);
			}
		},
		'onopen': {
			get: function() {
				return dc.getOnOpen();
			},
			set: function(cb) {
				dc.setOnOpen(cb);
			}
		},
		'onmessage': {
			get: function() {
				return dc.getOnMessage();
			},
			set: function(cb) {
				dc.setOnMessage(cb);
			}
		},
		'onclose': {
			get: function() {
				return dc.getOnClose();
			},
			set: function(cb) {
				dc.setOnClose(cb);
			}
		}
	});

	this.send = function send() {
		dc.send.apply(dc, arguments);
	};

	this.close = function close() {
		dc.close.apply(dc, arguments);
	};
};


function PeerConnection(configuration, constraints) {
	var that = this;
	this._pc = new _webrtc.PeerConnection(configuration, constraints);

	this._localType = null;
	this._remoteType = null;

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

	this._pc.onsignalingstatchange = function onsignalingstatchange(state) {
		stateString = that.RTCSignalingStates[state];
		if(that.onsignalingstatechange && typeof that.onsignalingstatechange == 'function') {
			onsignalingstatechange.apply(that, [stateString]);
		}
	};

	this._pc.oniceconnectionstatechange = function oniceconnectionstatechange(state) {
		stateString = that.RTCSignalingStates[state];
		if(that.oniceconnectionstatechange && typeof that.oniceconnectionstatechange == 'function') {
			oniceconnectionstatechange.apply(that, [stateString]);
		}
	};

	this._pc.onicegatheringstatechange = function onicegatheringstatechange(state) {
		stateString = that.RTCSignalingStates[state];
		if(that.onicegatheringstatechange && typeof that.onicegatheringstatechange == 'function') {
			onicegatheringstatechange.apply(that, [stateString]);
		}
	};

	this._pc.ondatachannel = function ondatachannel(internalDC) {
		if(that.ondatachannel && typeof that.ondatachannel == 'function') {
			var dc = new RTCDataChannel(internalDC);
			that.ondatachannel.apply(that, [dc]);
		}
	};

	this.onicecandidate = null;
	this.onsignalingstatechange = null;
	this.onicegatheringstatechange = null;
	this.ondatachannel = null;
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
