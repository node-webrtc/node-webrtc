function RTCDataChannelEvent(type, eventInitDict) {
  'use strict';
  Object.defineProperties(this, {
    'type': {
      value: type,
      enumerable: true
    },
    'channel': {
      value: eventInitDict.channel,
      enumerable: true
    }
  });
}

module.exports = RTCDataChannelEvent;
