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

  internalDC.onstatechange = function onstatechange(data) {
    switch (data) {
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
    maxRetransmits: {
      get: function getMaxRetransmits() {
        return internalDC.maxRetransmits;
      }
    },
    ordered: {
      get: function getOrdered() {
        return internalDC.ordered;
      }
    },
    priority: {
      get: function getPriority() {
        return internalDC.priority;
      }
    },
    protocol: {
      get: function getProtocol() {
        return internalDC.protocol;
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
    // NOTE(mroberts): Here's a hack to support jsdom's Blob implementation.
    var implSymbol = Object.getOwnPropertySymbols(data).find(function(symbol) {
      return symbol.toString() === 'Symbol(impl)';
    });
    if (data[implSymbol] && data[implSymbol]._buffer) {
      data = data[implSymbol]._buffer;
    }

    try {
      internalDC.send(data);
    } catch (error) {
      // TODO(mroberts): Start using the domexception library.
      if (error.message.match(/InvalidStateError/)) {
        error.code = 11;
        error.name = 'InvalidStateError';
      }
      throw error;
    }
  };

  this.close = function close() {
    internalDC.close();
  };
}

module.exports = RTCDataChannel;
