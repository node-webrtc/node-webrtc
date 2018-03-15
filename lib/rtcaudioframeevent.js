function RTCAudioFrameEvent(type, eventInitDict) {
    'use strict';
    Object.defineProperties(this, {
        'type': {
          value: type,
          enumerable: true
        },
        'id': {
            value: eventInitDict.id,
            enumerable: true
        },
        'bits_per_sample': {
            value: eventInitDict.bits_per_sample,
            enumerable: false
        },
        'sample_rate': {
            value: eventInitDict.sample_rate,
            enumerable: false
        },
        'number_of_channels': {
            value: eventInitDict.number_of_channels,
            enumberable: false
        },
        'number_of_frames': {
            value: eventInitDict.number_of_frames,
            enumerable: false
        },
        'buffer': {
            value: eventInitDict.buffer,
            enumerable: true
        }
    });
}

module.exports = RTCAudioFrameEvent;
