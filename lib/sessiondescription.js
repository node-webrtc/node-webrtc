'use strict';

function RTCSessionDescription(descriptionInitDict) {
  if (descriptionInitDict) {
    this.type = descriptionInitDict.type;
    this.sdp = descriptionInitDict.sdp;
  }

  this.toJSON = () => {
    const { sdp, type } = this;

    return {
      sdp,
      type
    };
  };
}

module.exports = RTCSessionDescription;
