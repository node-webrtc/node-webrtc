function MediaStreamTrack(internalMST) {
  'use strict';
  var that = this;
  this._mst = internalMST;

  this._queue = [];
  this._pending = null;

  this._mst.onmute = function onmute() {
    if(that.onmute && typeof that.onmute === 'function') {
      that.onmute.apply(that, []);
    }
  };

  this._mst.onunmute = function onunmute() {
    if(that.onunmute && typeof that.onunmute === 'function') {
      that.onunmute.apply(that, []);
    }
  };

  this._mst.onstarted = function onstarted() {
    if(that.onstarted && typeof that.onstarted === 'function') {
      that.onstarted.apply(that, []);
    }
  };

  this._mst.onended = function onended() {
    if(that.onended && typeof that.onended === 'function') {
      that.onended.apply(that, []);
    }
  };

  this.onmute = null;
  this.onunmute = null;
  this.onstarted = null;
  this.onstarted = null;
}

module.exports = MediaStreamTrack;

MediaStreamTrack.prototype._getMST = function _getMST() {
  'use strict';
  if(!this._mst) {
    throw new Error('RTCMediaSteamTrack is gone');
  }
  return this._mst;
};

MediaStreamTrack.prototype._queueOrRun = function _queueOrRun(obj) {
  'use strict';
  var mst = this._getMST();
  if(null === this._pending) {
    mst[obj.func].apply(mst, obj.args);
    if(obj.wait) {
      this._pending = obj;
    }
  } else {
    this._queue.push(obj);
  }
};

MediaStreamTrack.prototype._executeNext = function _executeNext() {
  'use strict';
  var obj, mst;
  mst = this._getMST();
  if(this._queue.length > 0) {
    obj = this._queue.shift();
    mst[obj.func].apply(mst, obj.args);
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

MediaStreamTrack.prototype.RTCMediaStreamTrackStates = [
  'initializing',
  'live',
  'ended',
  'failed'
];

MediaStreamTrack.prototype.getId = function getId() {
  'use strict';
  return this._getMST().id;
};

MediaStreamTrack.prototype.getKind = function getKind() {
  'use strict';
  return this._getMST().kind;
};

MediaStreamTrack.prototype.getLabel = function getLabel() {
  'use strict';
  return this._getMST().label;
};

MediaStreamTrack.prototype.getEnabled = function getEnabled() {
  'use strict';
  return this._getMST().enabled;
};

MediaStreamTrack.prototype.setEnabled = function setEnabled(enable) {
  'use strict';
  this._getMST().enabled = enable;
};

MediaStreamTrack.prototype.getMuted = function getMuted() {
  'use strict';
  return this._getMST().muted;
};

MediaStreamTrack.prototype.getReadOnly = function getReadOnly() {
  'use strict';
  return this._getMST()._readonly;
};

MediaStreamTrack.prototype.getRemote = function getRemote() {
  'use strict';
  return this._getMST().remote;
};

MediaStreamTrack.prototype.getReadyState = function getReadyState() {
  'use strict';
  var state = this._getMST().readyState;
  return this.RTCMediaStreamTrackStates[state];
};

// FIXME: implement stop
/*MediaStreamTrack.prototype.stop = function stop() {
  return this._getMST().stop();
};*/

// FIXME: implement clone
/*MediaStreamTrack.prototype.clone = function clone() {
  return this._getMST().clone();
};*/

MediaStreamTrack.prototype.getOnStarted = function() {
  'use strict';
  return this.onstarted;
};

MediaStreamTrack.prototype.setOnStarted = function(cb) {
  'use strict';
  // FIXME: throw an exception if cb isn't callable
  this.onstarted = cb;
};

MediaStreamTrack.prototype.getOnEnded = function() {
  'use strict';
  return this.onended;
};

MediaStreamTrack.prototype.setOnEnded = function(cb) {
  'use strict';
  // FIXME: throw an exception if cb isn't callable
  this.onended = cb;
};

MediaStreamTrack.prototype.getOnMute = function() {
  'use strict';
  return this.onmute;
};

MediaStreamTrack.prototype.setOnMute = function(cb) {
  'use strict';
  // FIXME: throw an exception if cb isn't callable
  this.onmute = cb;
};

MediaStreamTrack.prototype.getOnUnmute = function() {
  'use strict';
  return this.onunmute;
};

MediaStreamTrack.prototype.setOnUnmute = function(cb) {
  'use strict';
  // FIXME: throw an exception if cb isn't callable
  this.onunmute = cb;
};

function RTCMediaStreamTrack(internalMST) {
  'use strict';
  var mst = new MediaStreamTrack(internalMST);

  Object.defineProperties(this, {
    'id': {
      get: function getId() {
        return mst.getId();
      }
    },
    'label': {
      get: function getLabel() {
        return mst.getLabel();
      }
    },
    'kind': {
      get: function getKind() {
        return mst.getKind();
      }
    },
    'enabled': {
      get: function getEnabled() {
        return mst.getEnabled();
      },
      set: function setEnabled(enable) {
        mst.setEnabled(enable);
      }
    },
    'muted': {
      get: function getMuted() {
        return mst.getMuted();
      }
    },
    '_readonly': {
      get: function getReadOnly() {
        return mst.getReadOnly();
      }
    },
    'remote': {
      get: function getRemote() {
        return mst.getRemote();
      }
    },
    'readyState': {
      get: function getReadyState() {
        return mst.getReadyState();
      }
    },
    'onstarted': {
      get: function() {
        return mst.getOnStarted();
      },
      set: function(cb) {
        mst.setOnStarted(cb);
      }
    },
    'onended': {
      get: function() {
        return mst.getOnEnded();
      },
      set: function(cb) {
        mst.setOnEnded(cb);
      }
    },
    'onmute': {
      get: function() {
        return mst.getOnMute();
      },
      set: function(cb) {
        mst.setOnMute(cb);
      }
    },
    'onunmute': {
      get: function() {
        return mst.getOnUnmute();
      },
      set: function(cb) {
        mst.setOnUnmute(cb);
      }
    }
  });

  // FIXME: implement stop
  /*this.stop = function stop() {
    return mst.stop.apply(mst, arguments);
  };*/

  // FIXME: implement clone
  /*this.clone = function clone() {
    return mst.clone.apply(mst, arguments);
  };*/
}

module.exports = RTCMediaStreamTrack;
