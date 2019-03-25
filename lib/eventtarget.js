'use strict';

/**
 * @author mrdoob / http://mrdoob.com/
 * @author Jesús Leganés Combarro "Piranna" <piranna@gmail.com>
 */

function EventTarget() {
  this._listeners = {};
}

EventTarget.prototype.addEventListener = function addEventListener(type, listener) {
  var listeners = this._listeners = this._listeners || {};

  if (!listeners[type]) {
    listeners[type] = [];
  }

  if (listeners[type].indexOf(listener) === -1) {
    listeners[type].push(listener);
  }
};

EventTarget.prototype.dispatchEvent = function dispatchEvent(event) {
  var listeners = this._listeners = this._listeners || {};
  var self = this;

  process.nextTick(function() {
    var listenerArray = (listeners[event.type] || []);

    var dummyListener = self['on' + event.type];
    if (typeof dummyListener === 'function') {
      listenerArray = listenerArray.concat(dummyListener);
    }

    for (var i = 0, l = listenerArray.length, listener; i < l; i++) {
      listener = listenerArray[i];
      if (typeof listener === 'object' && typeof listener.handleEvent === 'function') {
        listener.handleEvent(event);
      } else {
        listener.call(self, event);
      }
    }
  });
};

EventTarget.prototype.removeEventListener = function removeEventListener(type, listener) {
  var listeners = this._listeners = this._listeners || {};
  var index = listeners[type].indexOf(listener);

  if (index !== -1) {
    listeners[type].splice(index, 1);
  }
};

module.exports = EventTarget;
