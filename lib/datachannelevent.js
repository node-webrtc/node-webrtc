'use strict';

function RTCDataChannelEvent(type, eventInitDict) {
  Object.defineProperties(this, {
    type: {
      value: type,
      enumerable: true
    },
    channel: {
      value: eventInitDict.channel,
      enumerable: true
    }
  });
}

module.exports = RTCDataChannelEvent;
