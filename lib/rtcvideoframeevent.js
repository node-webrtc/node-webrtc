function RTCVideoFrameEvent(type, eventInitDict) {
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
        'width': {
            value: eventInitDict.width,
            enumerable: false
        },
        'height': {
            value: eventInitDict.height,
            enumerable: false
        },
        'frameType': {
            value: eventInitDict.frameType
        },
        'timestamp': {
            value: eventInitDict.timestamp
        },
        'encodedFrame': {
            value: eventInitDict.encodedFrame,
            enumerable: true
        },
        'yPlane': {
            value: eventInitDict.yPlane,
            enumerable: true
        },
        'uPlane': {
            value: eventInitDict.uPlane,
            enumerable: true
        },
        'vPlane': {
            value: eventInitDict.vPlane,
            enumerable: true
        }
    });
}

module.exports = RTCVideoFrameEvent;
