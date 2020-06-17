'use strict';

var inherits = require('util').inherits;

const { getDisplayMedia, getUserMedia } = require('./binding');

var EventTarget = require('./eventtarget');

function MediaDevices() {}

inherits(MediaDevices, EventTarget);

MediaDevices.prototype.enumerateDevices = function enumerateDevices() {
  throw new Error('Not yet implemented; file a feature request against node-webrtc');
};

MediaDevices.prototype.getSupportedConstraints = function getSupportedConstraints() {
  throw new Error('Not yet implemented; file a feature request against node-webrtc');
};

MediaDevices.prototype.getDisplayMedia = getDisplayMedia;
MediaDevices.prototype.getUserMedia = getUserMedia;

module.exports = MediaDevices;
