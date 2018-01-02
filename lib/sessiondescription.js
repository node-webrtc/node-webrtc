'use strict';

function RTCSessionDescription(descriptionInitDict) {
  if (descriptionInitDict) {
    this.type = descriptionInitDict.type;
    this.sdp = descriptionInitDict.sdp;
  }
}

module.exports = RTCSessionDescription;
