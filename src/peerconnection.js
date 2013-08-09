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

function PeerConnection() {
	this._pc = new _webrtc.PeerConnection();

	this._localType = null;
	this._remoteType = null;

	this._iceGatheringState = 'new';
	this._iceConnectionState = 'new';
};

PeerConnection.prototype._signalingStateMap = [
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
	return this._signalingStateMap[this._getPC().signalingState];
};

PeerConnection.prototype.getIceGatheringState = function getIceGatheringState() {
	return this._iceGatheringState;
};

PeerConnection.prototype.getIceConnectionState = function getIceConnectionState() {
	return this._iceConnectionState;
};

PeerConnection.prototype.createOffer = function createOffer() {

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
};

exports.RTCPeerConnection = RTCPeerConnection;
exports.RTCSessionDescription = RTCSessionDescription;
exports.RTCIceCandidate = RTCIceCandidate;
