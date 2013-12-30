function DataChannel(internalDC) {
  var that = this;
  this._dc = internalDC;

  this._queue = [];
  this._pending = null;

  this._dc.onerror = function onerror() {

  };

  this._dc.onmessage = function onmessage(data) {
    if(that.onmessage && typeof that.onmessage == 'function') {
      that.onmessage.apply(that, [data]);
    }
  };

  this._dc.onstatechange = function onstatechange() {
    var state = that.getReadyState();
    if('open' == state) {
      if(that.onopen && typeof that.onopen == 'function') {
        that.onopen.apply(that, []);
      }
    } else if('closed' == state) {
      if(that.onclose && typeof that.onclose == 'function') {
        that.onclose.apply(that, []);
      }
    }
  };

  this.onerror = null;
  this.onmessage = null;
  this.onopen = null;
  this.onclose = null;
};

module.exports = DataChannel;

DataChannel.prototype._getDC = function _getDC() {
  if(!this._dc) {
    throw new Error('RTCDataChannel is gone');
  }
  return this._dc;
};

DataChannel.prototype._queueOrRun = function _queueOrRun(obj) {
  var pc = this._getPC();
  // this._checkClosed();
  if(null == this._pending) {
    pc[obj.func].apply(pc, obj.args);
    if(obj.wait) {
      this._pending = obj;
    }
  } else {
    this._queue.push(obj);
  }
};

DataChannel.prototype._executeNext = function _executeNext() {
  var obj, pc;
  pc = this._getPC();
  if(this._queue.length > 0) {
    obj = this._queue.shift();
    pc[obj.func].apply(pc, obj.args);
    if(obj.wait)
    {
      this._pending = obj;
    } else {
      this._executeNext();
    }
  } else {
    this._pending = null;
  }
};

DataChannel.prototype.RTCDataStates = [
  'connecting',
  'open',
  'closing',
  'closed'
];

DataChannel.prototype.BinaryTypes = [
  'blob', // Note: not sure what to do about this, since node doesn't have a Blob API
  'arraybuffer'
];

DataChannel.prototype.send = function send(data) {
  this._getDC().send(data);
};

DataChannel.prototype.close = function close() {
  this._getDC().close();
};

DataChannel.prototype._shutdown = function _shutdown() {
  this._getDC().shutdown();
};

DataChannel.prototype.getLabel = function getLabel() {
  return this._getDC().label;
};

DataChannel.prototype.getReadyState = function getReadyState() {
  var state = this._getDC().readyState;
  return this.RTCDataStates[state];
};

DataChannel.prototype.getBinaryType = function getBinaryType() {
  var type = this._getDC().binaryType;
  return this.BinaryTypes[type];
};

DataChannel.prototype.setBinaryType = function setBinaryType(type) {
  var typenum = this.BinaryTypes.indexOf(type);
  if(typenum >= 0) {
    this._getDC().binaryType = typenum;
  }
};

DataChannel.prototype.getOnError = function() {
  return this.onerror;
};

DataChannel.prototype.setOnError = function(cb) {
  // FIXME: throw an exception if cb isn't callable
  this.onerror = cb;
};

DataChannel.prototype.getOnOpen = function() {
  return this.onopen;
};

DataChannel.prototype.setOnOpen = function(cb) {
  // FIXME: throw an exception if cb isn't callable
  this.onopen = cb;
};

DataChannel.prototype.getOnMessage = function() {
  return this.onmessage;
};

DataChannel.prototype.setOnMessage = function(cb) {
  // FIXME: throw an exception if cb isn't callable
  this.onmessage = cb;
};

DataChannel.prototype.getOnClose = function() {
  return this.onclose;
};

DataChannel.prototype.setOnClose = function(cb) {
  // FIXME: throw an exception if cb isn't callable
  this.onclose = cb;
};


function RTCDataChannel(internalDC) {
  var dc = new DataChannel(internalDC);

  Object.defineProperties(this, {
    'label': {
      get: function getLabel() {
        return dc.getLabel();
      }
    },
    'readyState': {
      get: function getReadyState() {
        return dc.getReadyState();
      }
    },
    'binaryType': {
      get: function getBinaryType() {
        return dc.getBinaryType();
      },
      set: function(type) {
        dc.setBinaryType(type);
      }
    },
    'onerror': {
      get: function() {
        return dc.getOnError();
      },
      set: function(cb) {
        dc.setOnError(cb);
      }
    },
    'onopen': {
      get: function() {
        return dc.getOnOpen();
      },
      set: function(cb) {
        dc.setOnOpen(cb);
      }
    },
    'onmessage': {
      get: function() {
        return dc.getOnMessage();
      },
      set: function(cb) {
        dc.setOnMessage(cb);
      }
    },
    'onclose': {
      get: function() {
        return dc.getOnClose();
      },
      set: function(cb) {
        dc.setOnClose(cb);
      }
    }
  });

  this.send = function send() {
    dc.send.apply(dc, arguments);
  };

  this.close = function close() {
    dc.close.apply(dc, arguments);
  };
};

module.exports = RTCDataChannel;