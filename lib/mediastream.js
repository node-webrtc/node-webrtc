var RTCMediaStreamTrack = require('./mediastreamtrack');

function MediaStream(internalMS) {
  'use strict';
  var that = this;
  this._ms = internalMS;

  this._queue = [];
  this._pending = null;

  this._ms.onactive = function onactive() {
    if(that.onactive && typeof that.onactive === 'function') {
      that.onactive.apply(that, []);
    }
  };

  this._ms.oninactive = function oninactive() {
    if(that.oninactive && typeof that.oninactive === 'function') {
      that.oninactive.apply(that, []);
    }
  };

  this._ms.onaddtrack = function onaddtrack(internalMST) {
    if(that.onaddtrack && typeof that.onaddtrack === 'function') {
      var mst = new RTCMediaStreamTrack(internalMST);
      that.onaddtrack.apply(that, [mst]);
    }
  };

  this._ms.onremovetrack = function onremovetrack(internalMST) {
    if(that.onremovetrack && typeof that.onremovetrack === 'function') {
      var mst = new RTCMediaStreamTrack(internalMST);
      that.onremovetrack.apply(that, [mst]);
    }
  };

  this.onactive = null;
  this.oninactive = null;
  this.onaddtrack = null;
  this.onremovetrack = null;
}

module.exports = MediaStream;

MediaStream.prototype._getMS = function _getMS() {
  'use strict';
  if(!this._ms) {
    throw new Error('RTCMediaSteam is gone');
  }
  return this._ms;
};

MediaStream.prototype._queueOrRun = function _queueOrRun(obj) {
  'use strict';
  var ms = this._getMS();
  if(null === this._pending) {
    ms[obj.func].apply(ms, obj.args);
    if(obj.wait) {
      this._pending = obj;
    }
  } else {
    this._queue.push(obj);
  }
};

MediaStream.prototype._executeNext = function _executeNext() {
  'use strict';
  var obj, ms;
  ms = this._getMS();
  if(this._queue.length > 0) {
    obj = this._queue.shift();
    ms[obj.func].apply(ms, obj.args);
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

MediaStream.prototype.getId = function getId() {
  'use strict';
  return this._runImmediately({
    func: 'getId',
    args: []
  });
};

MediaStream.prototype.isInactive = function isInactive() {
  'use strict';
  return this._runImmediately({
    func: 'isInactive',
    args: []
  });
};

MediaStream.prototype.getaudiotracks = function getaudiotracks() {
  'use strict';
  return this._runImmediately({
    func: 'getAudioTracks',
    args: []
  });
};

MediaStream.prototype.getvideotracks = function getvideotracks() {
  'use strict';
  return this._runImmediately({
    func: 'getVideoTracks',
    args: []
  });
};

MediaStream.prototype.gettrackbyid = function gettrackbyid(id) {
  'use strict';
  return this._runImmediately({
    func: 'getTrackById',
    args: [id]
  });
};

MediaStream.prototype.addtrack = function addtrack(track) {
  'use strict';
  this._queueOrRun({
    func: 'addTrack',
    args: [track._getMST()]
  });
};

MediaStream.prototype.removetrack = function removetrack(track) {
  'use strict';
  this._queueOrRun({
    func: 'removeTrack',
    args: [track._getMST()]
  });
};

// FIXME: implement clone
/*MediaStream.prototype.clone = function clone() {
  return this._getMS().clone();
};*/

MediaStream.prototype.getOnActive = function() {
  'use strict';
  return this.onactive;
};

MediaStream.prototype.setOnActive = function(cb) {
  'use strict';
  // FIXME: throw an exception if cb isn't callable
  this.onactive = cb;
};

MediaStream.prototype.getOnInactive = function() {
  'use strict';
  return this.oninactive;
};

MediaStream.prototype.setOnInactive = function(cb) {
  'use strict';
  // FIXME: throw an exception if cb isn't callable
  this.oninactive = cb;
};

MediaStream.prototype.getOnAddTrack = function() {
  'use strict';
  return this.onaddtrack;
};

MediaStream.prototype.setOnAddTrack = function(cb) {
  'use strict';
  // FIXME: throw an exception if cb isn't callable
  this.onaddtrack = cb;
};

MediaStream.prototype.getOnRemoveTrack = function() {
  'use strict';
  return this.onremovetrack;
};

MediaStream.prototype.setOnRemoveTrack = function(cb) {
  'use strict';
  // FIXME: throw an exception if cb isn't callable
  this.onremovetrack = cb;
};

function RTCMediaStream(internalMS) {
  'use strict';
  var ms = new MediaStream(internalMS);

  Object.defineProperties(this, {
    'id': {
      get: function getId() {
        return ms.getId();
      }
    },
    'inactive': {
      get: function isInactive() {
        return ms.isInactive();
      }
    },
    'onaddtrack': {
      get: function() {
        return ms.getOnAddTrack();
      },
      set: function(cb) {
        ms.setOnAddTrack(cb);
      }
    },
    'onremovetrack': {
      get: function() {
        return ms.getOnRemoveTrack();
      },
      set: function(cb) {
        ms.setOnRemoveTrack(cb);
      }
    },
    'onactive': {
      get: function() {
        return ms.getOnActive();
      },
      set: function(cb) {
        ms.setOnActive(cb);
      }
    },
    'oninactive': {
      get: function() {
        return ms.getOnInactive();
      },
      set: function(cb) {
        ms.setOnInactive(cb);
      }
    }
  });

  this.getaudiotracks = function getaudiotracks() {
    return ms.getaudiotracks.apply(ms, arguments);
  };

  this.getvideotracks = function getvideotracks() {
    return ms.getvideotracks.apply(ms, arguments);
  };

  this.gettrackbyid = function gettrackbyid() {
    return ms.gettrackbyid.apply(ms, arguments);
  };

  this.addtrack = function addtrack() {
    ms.addtrack.apply(ms, arguments);
  };

  this.removetrack = function removetrack() {
    ms.removetrack.apply(ms, arguments);
  };

  // FIXME: implement clone
  /*this.clone = function clone() {
    return ms.clone.apply(ms, arguments);
  };*/
}

module.exports = RTCMediaStream;
