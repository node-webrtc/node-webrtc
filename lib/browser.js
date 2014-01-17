var RTCIceCandidate       = mozRTCIceCandidate       || webkitRTCIceCandidate       || RTCIceCandidate;
var RTCPeerConnection     = mozRTCPeerConnection     || webkitRTCPeerConnection     || RTCPeerConnection;
var RTCSessionDescription = mozRTCSessionDescription || webkitRTCSessionDescription || RTCSessionDescription;


exports.RTCIceCandidate       = RTCIceCandidate;
exports.RTCPeerConnection     = RTCPeerConnection;
exports.RTCSessionDescription = RTCSessionDescription;