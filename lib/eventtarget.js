'use strict';

/**
 * @author mrdoob / http://mrdoob.com/
 * @author Jesús Leganés Combarro "Piranna" <piranna@gmail.com>
 */

function EventTarget() {
  this._listeners = {};
}

EventTarget.prototype.addEventListener = function addEventListener(type, listener) {
  const listeners = this._listeners = this._listeners || {};

  if (!listeners[type]) {
    listeners[type] = new Set();
  }

  listeners[type].add(listener);
};

EventTarget.prototype.dispatchEvent = function dispatchEvent(event) {
  let listeners = this._listeners = this._listeners || {};

  process.nextTick(() => {
    listeners = new Set(listeners[event.type] || []);

    const dummyListener = this['on' + event.type];
    if (typeof dummyListener === 'function') {
      listeners.add(dummyListener);
    }

    listeners.forEach(listener => {
      if (typeof listener === 'object' && typeof listener.handleEvent === 'function') {
        listener.handleEvent(event);
      } else {
        listener.call(this, event);
      }
    });
  });
};

EventTarget.prototype.removeEventListener = function removeEventListener(type, listener) {
  const listeners = this._listeners = this._listeners || {};
  if (listeners[type]) {
    listeners[type].delete(listener);
  }
};

module.exports = EventTarget;
