'use strict';

const { inherits } = require('util');

const {
  MediaStream,
  MediaStreamTrack,
  RTCAudioSource,
  RTCDtlsTransport,
  RTCIceTransport,
  RTCRtpReceiver,
  RTCRtpSender,
  RTCRtpTransceiver,
  RTCVideoSource,
  getUserMedia,
  i420ToRgba,
  rgbaToI420,
  setDOMException
} = require('./binding');

const EventTarget = require('./eventtarget');

inherits(MediaStream, EventTarget);
inherits(MediaStreamTrack, EventTarget);
inherits(RTCDtlsTransport, EventTarget);
inherits(RTCIceTransport, EventTarget);

try {
  setDOMException(require('domexception'));
} catch (error) {
  // Do nothing
}

const nonstandard = {
  i420ToRgba,
  RTCAudioSink: require('./rtcaudiosink'),
  RTCAudioSource,
  RTCVideoSink: require('./rtcvideosink'),
  RTCVideoSource,
  rgbaToI420
};

module.exports = {
  MediaStream,
  MediaStreamTrack,
  RTCDataChannel: require('./datachannel'),
  RTCDataChannelEvent: require('./datachannelevent'),
  RTCDtlsTransport,
  RTCIceCandidate: require('./icecandidate'),
  RTCIceTransport,
  RTCPeerConnection: require('./peerconnection'),
  RTCPeerConnectionIceEvent: require('./rtcpeerconnectioniceevent'),
  RTCRtpReceiver,
  RTCRtpSender,
  RTCRtpTransceiver,
  RTCSessionDescription: require('./sessiondescription'),
  getUserMedia,
  nonstandard,
};
