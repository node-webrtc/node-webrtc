'use strict';

function RTCDataChannelEvent(type, eventInitDict) {
  Object.defineProperties(this, {
    bubbles: {
      value: false
    },
    cancelable: {
      value: false
    },
    type: {
      value: type,
      enumerable: true
    },
    channel: {
      value: eventInitDict.channel,
      enumerable: true
    },
    target: {
      value: eventInitDict.target,
      enumerable: true
    }
  });
}

module.exports = RTCDataChannelEvent;
