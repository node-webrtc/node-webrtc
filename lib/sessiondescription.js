function RTCSessionDescription(descriptionInitDict) {
  'use strict';
  if(descriptionInitDict) {
    this.type = descriptionInitDict.type;
    this.sdp = descriptionInitDict.sdp;
  }
}

module.exports = RTCSessionDescription;
