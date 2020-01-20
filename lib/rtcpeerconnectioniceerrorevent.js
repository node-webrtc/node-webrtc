'use strict';

function RTCPeerConnectionIceErrorEvent(type, eventInitDict) {
  Object.defineProperties(this, {
    type: {
      value: type,
      enumerable: true
    },
    address: {
      value: eventInitDict.address,
      enumerable: true
    },
    port: {
      value: eventInitDict.port,
      enumerable: true
    },
    url: {
      value: eventInitDict.url,
      enumerable: true
    },
    errorCode: {
      value: eventInitDict.errorCode,
      enumerable: true
    },
    errorText: {
      value: eventInitDict.errorText,
      enumerable: true
    }
  });
}

module.exports = RTCPeerConnectionIceErrorEvent;
