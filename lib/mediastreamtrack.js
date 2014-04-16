var EventTarget = require('./eventtarget');

//function MediaStreamTrack(internalMST) {
//  this._queue = [];
//  this._pending = null;
//}

//MediaStreamTrack.prototype._getMST = function _getMST() {
//  if(!this._mst) {
//    throw new Error('RTCMediaSteamTrack is gone');
//  }
//  return this._mst;
//};

//MediaStreamTrack.prototype._queueOrRun = function _queueOrRun(obj) {
//  var mst = this._getMST();
//  if(null == this._pending) {
//    mst[obj.func].apply(mst, obj.args);
//    if(obj.wait) {
//      this._pending = obj;
//    }
//  } else {
//    this._queue.push(obj);
//  }
//};

//MediaStreamTrack.prototype._executeNext = function _executeNext() {
//  var obj, mst;
//  mst = this._getMST();
//  if(this._queue.length > 0) {
//    obj = this._queue.shift();
//    mst[obj.func].apply(mst, obj.args);
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

// FIXME: implement stop
/*MediaStreamTrack.prototype.stop = function stop() {
  return this._getMST().stop();
};*/

// FIXME: implement clone
/*MediaStreamTrack.prototype.clone = function clone() {
  return this._getMST().clone();
};*/


function MediaStreamTrack(internalMST) {
  'use strict';
  var that = this;

  EventTarget.call(this);

  internalMST.onmute = function onmute() {
    that.dispatchEvent({type: 'mute'});
  };

  internalMST.onunmute = function onunmute() {
    that.dispatchEvent({type: 'unmute'});
  };

  internalMST.onstarted = function onstarted() {
    that.dispatchEvent({type: 'started'});
  };

  internalMST.onended = function onended() {
    that.dispatchEvent({type: 'ended'});
  };

  // [ToDo] onoverconstrained

  Object.defineProperties(this, {
    'id': {
      get: function getId() {
        return internalMST.id;
      }
    },
    'label': {
      get: function getLabel() {
        return internalMST.label;
      }
    },
    'kind': {
      get: function getKind() {
        return internalMST.kind;
      }
    },
    'enabled': {
      get: function getEnabled() {
        return internalMST.enabled;
      },
      set: function setEnabled(enable) {
        internalMST.enabled = enable;
      }
    },
    'muted': {
      get: function getMuted() {
        return internalMST.muted;
      }
    },
    '_readonly': {
      get: function getReadOnly() {
        return internalMST._readonly;
      }
    },
    'remote': {
      get: function getRemote() {
        return internalMST.remote;
      }
    },
    'readyState': {
      get: function getReadyState() {
        var state = internalMST.readyState;
        return this.MediaStreamTrackStates[state];
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

MediaStreamTrack.prototype.MediaStreamTrackStates = [
  'initializing',
  'live',
  'ended',
  'failed'
];

module.exports = MediaStreamTrack;
