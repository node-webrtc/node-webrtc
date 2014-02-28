function MediaStreamTrackEvent(type, eventInitDict) {
  'use strict';
  Object.defineProperties(this, {
    'type': {
      value: type,
      enumerable: true
    },
    'track': {
      value: eventInitDict.track,
      enumerable: true
    }
  });
}

module.exports = MediaStreamTrackEvent;
