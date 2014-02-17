var EventTarget = require('eventtarget');

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
  var that = this;

  internalDC.onerror = function onerror() {

  };

  internalDC.onmessage = function onmessage(data) {
    if(that.onmessage && typeof that.onmessage == 'function') {
       that.onmessage(new RTCDataChannelMessageEvent(data));
    }
  };

  internalDC.onstatechange = function onstatechange() {
    var state = that.readyState;

    if('open' == state) {
      if(that.onopen && typeof that.onopen == 'function') {
         that.onopen();
      }
    } else if('closed' == state) {
      if(that.onclose && typeof that.onclose == 'function') {
         that.onclose();
      }
    }
  };


  var onerror   = null;
  var onmessage = null;
  var onopen    = null;
  var onclose   = null;


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
    },

    'onerror': {
      get: function() {
        return onerror;
      },
      set: function(cb) {
        // FIXME: throw an exception if cb isn't callable
        onerror = cb;
      }
    },
    'onopen': {
      get: function() {
        return onopen;
      },
      set: function(cb) {
        // FIXME: throw an exception if cb isn't callable
        onopen = cb;
      }
    },
    'onmessage': {
      get: function() {
        return onmessage;
      },
      set: function(cb) {
        // FIXME: throw an exception if cb isn't callable
        onmessage = cb;
      }
    },
    'onclose': {
      get: function() {
        return onclose;
      },
      set: function(cb) {
        // FIXME: throw an exception if cb isn't callable
        onclose = cb;
      }
    }
  });

  this.send = function send(data) {
    internalDC.send(data);
  };

  this.close = function close() {
    internalDC.close();
  };
};

RTCDataChannel.prototype.RTCDataStates =
[
  'connecting',
  'open',
  'closing',
  'closed'
];

RTCDataChannel.prototype.BinaryTypes =
[
  'blob',  // Note: not sure what to do about this, since node doesn't have a Blob API
  'arraybuffer'
];


module.exports = RTCDataChannel;