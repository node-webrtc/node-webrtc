'use strict';

/**
 * @author mrdoob / http://mrdoob.com/
 * @author Jesús Leganés Combarro "Piranna" <piranna@gmail.com>
 */

function EventTarget() {
  var listeners = {};

  this.addEventListener = function addEventListener(type, listener) {
    if (!listeners[type]) {
      listeners[type] = [];
    }

    if (listeners[type].indexOf(listener) === -1) {
      listeners[type].push(listener);
    }
  };

  this.dispatchEvent = function dispatchEvent(event) {
    var self = this;
    process.nextTick(function() {
      var listenerArray = (listeners[event.type] || []);

      var dummyListener = self['on' + event.type];
      if (typeof dummyListener === 'function') {
        listenerArray = listenerArray.concat(dummyListener);
      }

      for (var i = 0, l = listenerArray.length; i < l; i++) {
        listenerArray[i].call(self, event);
      }
    });
  };

  this.removeEventListener = function removeEventListener(type, listener) {
    var index = listeners[type].indexOf(listener);

    if (index !== -1) {
      listeners[type].splice(index, 1);
    }
  };
}

module.exports = EventTarget;
