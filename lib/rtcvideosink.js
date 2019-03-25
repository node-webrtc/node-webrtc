'use strict';

var inherits = require('util').inherits;

var NativeRTCVideoSink = require('./binding').RTCVideoSink;
var EventTarget = require('./eventtarget');

function RTCVideoSink(track) {
  EventTarget.call(this);

  this._sink = new NativeRTCVideoSink(track);

  var self = this;
  this._sink.onframe = function onframe(frame) {
    self.dispatchEvent({
      type: 'frame',
      frame: frame
    });
  };

  Object.defineProperty(this, 'stopped', {
    get: function() {
      return self._sink.stopped;
    }
  });
}

inherits(RTCVideoSink, EventTarget);

RTCVideoSink.prototype.stop = function stop() {
  return this._sink.stop();
};

module.exports = RTCVideoSink;
