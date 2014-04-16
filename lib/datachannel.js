var EventTarget = require('./eventtarget');

var RTCDataChannelMessageEvent = require('./datachannelmessageevent');

//function DataChannel(internalDC) {
//  this._queue = [];
//  this._pending = null;
//};

//DataChannel.prototype._getDC = function _getDC() {
//  if(!this._dc) {
//    throw new Error('RTCDataChannel is gone');
//  }
//  return this._dc;
//};

//DataChannel.prototype._queueOrRun = function _queueOrRun(obj) {
//  var pc = this._getPC();
//  // this._checkClosed();
//  if(null == this._pending) {
//    pc[obj.func].apply(pc, obj.args);
//    if(obj.wait) {
//      this._pending = obj;
//    }
//  } else {
//    this._queue.push(obj);
//  }
//};

//DataChannel.prototype._executeNext = function _executeNext() {
//  var obj, pc;
//  pc = this._getPC();
//  if(this._queue.length > 0) {
//    obj = this._queue.shift();
//    pc[obj.func].apply(pc, obj.args);
//    if(obj.wait)
//    {
//      this._pending = obj;
//    } else {
//      this._executeNext();
//    }
//  } else {
//    this._pending = null;
//  }
//};

//DataChannel.prototype._shutdown = function _shutdown() {
//  this._getDC().shutdown();
//};


function RTCDataChannel(internalDC) {
  'use strict';
  var that = this;

  EventTarget.call(this);

  internalDC.onerror = function onerror() {
    that.dispatchEvent({type: 'error'});
  };

  internalDC.onmessage = function onmessage(data) {
    that.dispatchEvent(new RTCDataChannelMessageEvent(data));
  };

  internalDC.onstatechange = function onstatechange() {
    var state = that.readyState;

    switch(state) {
      case 'open':
        that.dispatchEvent({type: 'open'});
        break;

      case 'closed':
        that.dispatchEvent({type: 'close'});
        break;
    }
  };

  Object.defineProperties(this, {
    'label': {
      get: function getLabel() {
        return internalDC.label;
      }
    },
    'readyState': {
      get: function getReadyState() {
        var state = internalDC.readyState;
        return this.RTCDataStates[state];
      }
    },
    'binaryType': {
      get: function getBinaryType() {
        var type = internalDC.binaryType;
        return this.BinaryTypes[type];
      },
      set: function(type) {
        var typenum = this.BinaryTypes.indexOf(type);
        if(typenum >= 0) {
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
