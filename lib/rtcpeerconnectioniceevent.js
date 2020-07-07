'use strict';

function RTCPeerConnectionIceEvent(type, eventInitDict) {
  Object.defineProperties(this, {
    type: {
      value: type,
      enumerable: true
    },
    candidate: {
      value: eventInitDict.candidate,
      enumerable: true
    },
    srcElement: {
      value: eventInitDict.srcElement,
      enumerable: true
    }
  });
}

module.exports = RTCPeerConnectionIceEvent;
