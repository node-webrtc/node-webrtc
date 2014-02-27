function MediaStreamEvent(type, eventInitDict)
{
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
};


module.exports = MediaStreamEvent;