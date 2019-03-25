'use strict';

var inherits = require('util').inherits;

var NativeRTCAudioSink = require('./binding').RTCAudioSink;
var EventTarget = require('./eventtarget');

function RTCAudioSink(track) {
  EventTarget.call(this);

  this._sink = new NativeRTCAudioSink(track);

  var self = this;
  this._sink.ondata = function ondata(data) {
    self.dispatchEvent(Object.assign({
      type: 'data',
    }, data));
  };

  Object.defineProperty(this, 'stopped', {
    get: function() {
      return self._sink.stopped;
    }
  });
}

inherits(RTCAudioSink, EventTarget);

RTCAudioSink.prototype.stop = function stop() {
  return this._sink.stop();
};

module.exports = RTCAudioSink;
