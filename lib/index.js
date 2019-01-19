'use strict';

var binding = require('./binding');

exports.getUserMedia = binding.getUserMedia;
exports.MediaStream = binding.MediaStream;
exports.MediaStreamTrack = binding.MediaStreamTrack;
exports.RTCDataChannel = require('./datachannel');
exports.RTCDataChannelEvent = require('./datachannelevent');
exports.RTCIceCandidate = require('./icecandidate');
exports.RTCPeerConnection = require('./peerconnection');
exports.RTCPeerConnectionIceEvent = require('./rtcpeerconnectioniceevent');
exports.RTCRtpReceiver = binding.RTCRtpReceiver;
exports.RTCRtpSender = binding.RTCRtpSender;
exports.RTCRtpTransceiver = binding.RTCRtpTransceiver;
exports.RTCSessionDescription = require('./sessiondescription');

exports.nonstandard = {};
exports.nonstandard.RTCVideoSink = binding.RTCVideoSink;
exports.nonstandard.RTCVideoSource = binding.RTCVideoSource;
exports.nonstandard.argb32ToI420 = binding.argb32ToI420;
exports.nonstandard.i420ToArgb32 = binding.i420ToArgb32;
