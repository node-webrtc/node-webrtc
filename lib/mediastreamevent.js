function MediaStreamEvent(type, eventInitDict) {
  'use strict';
  Object.defineProperties(this, {
    'type': {
      value: type,
      enumerable: true
    },
    'stream': {
      value: eventInitDict.stream,
      enumerable: true
    }
  });
}

module.exports = MediaStreamEvent;
