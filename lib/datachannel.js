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

  internalDC.onstatechange = function onstatechange(state) {
    state = self.RTCDataStates[state];
    switch (state) {
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
    label: {
      get: function getLabel() {
        return internalDC.label;
      }
    },
    readyState: {
      get: function getReadyState() {
        var state = internalDC.readyState;
        return this.RTCDataStates[state];
      }
    },
    binaryType: {
      get: function getBinaryType() {
        var type = internalDC.binaryType;
        return this.BinaryTypes[type];
      },
      set: function(type) {
        var typenum = this.BinaryTypes.indexOf(type);
        if (typenum >= 0) {
          internalDC.binaryType = typenum;
        }
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

RTCDataChannel.prototype.RTCDataStates = [
  'connecting',
  'open',
  'closing',
  'closed'
];

RTCDataChannel.prototype.BinaryTypes = [
  'blob',  // Note: not sure what to do about this, since node doesn't have a Blob API
  'arraybuffer'
];

module.exports = RTCDataChannel;
