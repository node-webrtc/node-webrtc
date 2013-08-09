var _webrtc = require('bindings')('webrtc.node');

function PeerConnection() {
	this._pc = new _webrtc.PeerConnection();
	var that = this;

	Object.defineProperty(this, 'localDescription', {
		get: function() {
			var _localDescription = that._pc.localDescription;
			return _localDescription;
		}
	})
};

PeerConnection.prototype.createOffer = function createOffer() {

};

module.exports = PeerConnection;