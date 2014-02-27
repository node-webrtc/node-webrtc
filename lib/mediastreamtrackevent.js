function MediaStreamTrackEvent(type, eventInitDict)
{
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
};


module.exports = MediaStreamTrackEvent;