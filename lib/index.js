'use strict';

var inherits = require('util').inherits;

var binding = require('./binding');
var EventTarget = require('./eventtarget');

var RTCDtlsTransport = binding.RTCDtlsTransport;
inherits(RTCDtlsTransport, EventTarget);

exports.getUserMedia = binding.getUserMedia;
exports.MediaStream = binding.MediaStream;
exports.MediaStreamTrack = binding.MediaStreamTrack;
exports.RTCDataChannel = require('./datachannel');
exports.RTCDataChannelEvent = require('./datachannelevent');
exports.RTCDtlsTransport = RTCDtlsTransport;
exports.RTCIceCandidate = require('./icecandidate');
exports.RTCPeerConnection = require('./peerconnection');
exports.RTCPeerConnectionIceEvent = require('./rtcpeerconnectioniceevent');
exports.RTCRtpReceiver = binding.RTCRtpReceiver;
exports.RTCRtpSender = binding.RTCRtpSender;
exports.RTCRtpTransceiver = binding.RTCRtpTransceiver;
exports.RTCSessionDescription = require('./sessiondescription');

exports.nonstandard = {};
exports.nonstandard.i420ToRgba = binding.i420ToRgba;
exports.nonstandard.RTCAudioSink = require('./rtcaudiosink');
exports.nonstandard.RTCAudioSource = binding.RTCAudioSource;
exports.nonstandard.RTCVideoSink = require('./rtcvideosink');
exports.nonstandard.RTCVideoSource = binding.RTCVideoSource;
exports.nonstandard.rgbaToI420 = binding.rgbaToI420;
