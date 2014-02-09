function RTCSessionDescription(dict) {
  'use strict';
  this.type = dict.type;
  this.sdp = dict.sdp;
}

module.exports = RTCSessionDescription;
