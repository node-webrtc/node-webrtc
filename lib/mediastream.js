var EventTarget = require('./eventtarget');

var MediaStreamTrackEvent = require('./mediastreamtrackevent');
var MediaStreamTrack      = require('./mediastreamtrack');


//MediaStream.prototype._executeNext = function _executeNext() {
//  var obj, ms;
//  ms = this._getMS();
//  if(this._queue.length > 0) {
//    obj = this._queue.shift();
//    ms[obj.func].apply(ms, obj.args);
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


function MediaStream(internalMS) {
  'use strict';
  var that = this
    , queue = []
    , pending = null;

  EventTarget.call(this);

  this._ms = internalMS;

//  internalMS.onactive = function onactive() {
//    that.dispatchEvent({type: 'active'});
//  };
//
//  internalMS.oninactive = function oninactive() {
//    that.dispatchEvent({type: 'inactive'});
//  };

  internalMS.onaddtrack = function onaddtrack(internalMST) {
    var mst = new MediaStreamTrack(internalMST);

    that.dispatchEvent(new MediaStreamTrackEvent('addtrack', {track: mst}));
  };

  internalMS.onremovetrack = function onremovetrack(internalMST) {
    var mst = new MediaStreamTrack(internalMST);

    that.dispatchEvent(new MediaStreamTrackEvent('removetrack', {track: mst}));
  };

  // [ToDo] onended

  function queueOrRun(obj) {
    if(null === that.pending) {
      internalMS[obj.func].apply(internalMS, obj.args);

      if(obj.wait) {
        pending = obj;
      }
    } else {
      queue.push(obj);
    }
  }

  function runImmediately(obj) {
    return internalMS[obj.func].apply(internalMS, obj.args);
  }

  Object.defineProperties(this, {
    'id': {
      get: function getId() {
        return runImmediately({
          func: 'getId',
          args: []
        });
      }
    },
    'inactive': {
      get: function isInactive() {
        return runImmediately({
          func: 'isInactive',
          args: []
        });
      }
    }
  });

  this.getAudioTracks = function getaudiotracks() {
    return runImmediately({
      func: 'getAudioTracks',
      args: []
    });
  };

  this.getVideoTracks = function getvideotracks() {
    return runImmediately({
      func: 'getVideoTracks',
      args: []
    });
  };

  this.getTrackById = function gettrackbyid(id) {
    return runImmediately({
      func: 'getTrackById',
      args: [id]
    });
  };

  this.addTrack = function addtrack(track) {
    queueOrRun({
      func: 'addTrack',
      args: [track._getMST()]
    });
  };

  this.removeTrack = function removetrack(track) {
    queueOrRun({
      func: 'removeTrack',
      args: [track._getMST()]
    });
  };

  // FIXME: implement clone
  /*this.clone = function clone() {
    return internalMS.clone();
  };*/
}

MediaStream.prototype._getMS = function _getMS() {
  'use strict';

  if(!this._ms) {
    throw new Error('RTCMediaSteam is gone');
  }
  return this._ms;
};

module.exports = MediaStream;
