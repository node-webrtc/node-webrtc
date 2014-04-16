function RTCPeerConnectionIceEvent(type, eventInitDict) {
  'use strict';
  Object.defineProperties(this, {
    'type': {
      value: type,
      enumerable: true
    },
    'candidate': {
      value: eventInitDict.candidate,
      enumerable: true
    }
  });
}

module.exports = RTCPeerConnectionIceEvent;
