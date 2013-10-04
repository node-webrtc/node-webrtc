var _webrtc = require('bindings')('webrtc.node');

function RTCIceCandidate(dict) {
	this.candidate = dict.candidate;
	this.sdpMid = dict.sdpMid;
	this.sdpMLineIndex = ('sdpMLineIndex' in dict) ? dict.sdpMLineIndex + 1 : null;
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


function PeerConnection() {
	this._pc = new _webrtc.PeerConnection();

	this._localType = null;
	this._remoteType = null;

	this._iceGatheringState = 'new';
	this._iceConnectionState = 'new';

	this._queue = [];
	this._pending = null;
};

PeerConnection.prototype.RTCSignalingStateMap = [
	'invalid',
	'stable',
	'have-local-offer',
	'have-remote-offer',
	'have-local-pranswer',
	'have-remote-pranswer',
	'closed'
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
	var pc;
	this._checkClosed();
	if(null == this._pending) {
		pc = this._getPC();
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
	if(this._queue.length > 0) {
		obj = this._queue.shift();
		pc = this._getPC();
		pc[obj.func].apply(pc, obj.args);
		if(!obj.wait)
		{
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
	return this.RTCSignalingStateMap[this._getPC().signalingState];
};

PeerConnection.prototype.getIceGatheringState = function getIceGatheringState() {
	return this._iceGatheringState;
};

PeerConnection.prototype.getIceConnectionState = function getIceConnectionState() {
	return this._iceConnectionState;
};

PeerConnection.prototype.createOffer = function createOffer(onSuccess, onError, constraints) {
	//this._getPC().createOffer(onSuccess, onError, constraints);
	constraints = constraints || {};
	// FIXME: complain if onError is undefined
	this._queueOrRun({
		func: 'createOffer',
		args: [constraints],
		wait: true,
		onSuccess: onSuccess,
		onError: onError
	});
};


function RTCPeerConnection() {
	var pc = new PeerConnection();

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
		'iceGatheringState': {
			get: function getIceGatheringState() {
				return pc.getIceGatheringState();
			}
		},
		'iceConnectionState': {
			get: function getIceConnectionState() {
				return pc.getIceConnectionState();
			}
		}
	});

	this.createOffer = function createOffer() {
		pc.createOffer.apply(pc, arguments);
	}
};

exports.RTCPeerConnection = RTCPeerConnection;
exports.RTCSessionDescription = RTCSessionDescription;
exports.RTCIceCandidate = RTCIceCandidate;
