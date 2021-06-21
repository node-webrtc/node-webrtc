'use strict';

function RTCSessionDescription(descriptionInitDict) {
  if (descriptionInitDict) {
    this.type = descriptionInitDict.type;
    this.sdp = descriptionInitDict.sdp;
  }

  this.toJSON = () => {
    const { type, sdp } = this;

    return { type, sdp };
  };
}

module.exports = RTCSessionDescription;
