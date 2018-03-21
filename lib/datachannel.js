'use strict';

var EventTarget = require('./eventtarget');

var RTCDataChannelMessageEvent = require('./datachannelmessageevent');

function RTCDataChannel(internalDC) {
  var self = this;

  EventTarget.call(this);

  internalDC.onerror = function onerror() {
    self.dispatchEvent({ type: 'error' });
  };

  internalDC.onmessage = function onmessage(data) {
    self.dispatchEvent(new RTCDataChannelMessageEvent(data));
  };

  internalDC.onstatechange = function onstatechange() {
    switch (this.readyState) {
      case 'open':
        self.dispatchEvent({ type: 'open' });
        break;

      case 'closed':
        self.dispatchEvent({ type: 'close' });
        break;
    }
  };

  Object.defineProperties(this, {
    bufferedAmount: {
      get: function getBufferedAmount() {
        return internalDC.bufferedAmount;
      }
    },
    id: {
      get: function getId() {
        return internalDC.id;
      }
    },
    label: {
      get: function getLabel() {
        return internalDC.label;
      }
    },
    readyState: {
      get: function getReadyState() {
        return internalDC.readyState;
      }
    },
    binaryType: {
      get: function getBinaryType() {
        return internalDC.binaryType;
      },
      set: function(binaryType) {
        internalDC.binaryType = binaryType;
      }
    }
  });

  this.send = function send(data) {
    internalDC.send(data);
  };

  this.close = function close() {
    internalDC.close();
  };
}

module.exports = RTCDataChannel;
