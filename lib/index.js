'use strict';

const { inherits } = require('util');

const {
  MediaStream,
  MediaStreamTrack,
  RTCAudioSink,
  RTCAudioSource,
  RTCDataChannel,
  RTCDtlsTransport,
  RTCIceTransport,
  RTCRtpReceiver,
  RTCRtpSender,
  RTCRtpTransceiver,
  RTCSctpTransport,
  RTCVideoSink,
  RTCVideoSource,
  getUserMedia,
  i420ToRgba,
  rgbaToI420,
  setDOMException
} = require('./binding');

const EventTarget = require('./eventtarget');
const MediaDevices = require('./mediadevices');

inherits(MediaStream, EventTarget);
inherits(MediaStreamTrack, EventTarget);
inherits(RTCAudioSink, EventTarget);
inherits(RTCDataChannel, EventTarget);
inherits(RTCDtlsTransport, EventTarget);
inherits(RTCIceTransport, EventTarget);
inherits(RTCSctpTransport, EventTarget);
inherits(RTCVideoSink, EventTarget);

try {
  setDOMException(require('domexception'));
} catch (error) {
  // Do nothing
}

// NOTE(mroberts): Here's a hack to support jsdom's Blob implementation.
RTCDataChannel.prototype.send = function send(data) {
  const implSymbol = Object.getOwnPropertySymbols(data).find(symbol => symbol.toString() === 'Symbol(impl)');
  if (data[implSymbol] && data[implSymbol]._buffer) {
    data = data[implSymbol]._buffer;
  }
  this._send(data);
};

const mediaDevices = new MediaDevices();

const nonstandard = {
  i420ToRgba,
  RTCAudioSink,
  RTCAudioSource,
  RTCVideoSink,
  RTCVideoSource,
  rgbaToI420
};

module.exports = {
  MediaStream,
  MediaStreamTrack,
  RTCDataChannel,
  RTCDataChannelEvent: require('./datachannelevent'),
  RTCDtlsTransport,
  RTCIceCandidate: require('./icecandidate'),
  RTCIceTransport,
  RTCPeerConnection: require('./peerconnection'),
  RTCPeerConnectionIceEvent: require('./rtcpeerconnectioniceevent'),
  RTCRtpReceiver,
  RTCRtpSender,
  RTCRtpTransceiver,
  RTCSctpTransport,
  RTCSessionDescription: require('./sessiondescription'),
  getUserMedia,
  mediaDevices,
  nonstandard,
};
