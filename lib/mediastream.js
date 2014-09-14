var EventTarget = require('./eventtarget');

var MediaStreamTrackEvent = require('./mediastreamtrackevent');
var MediaStreamTrack      = require('./mediastreamtrack');

function MediaStream(internalMS) {
  'use strict';
  var that = this
    , queue = []
    , pending = null;

  EventTarget.call(this);

  internalMS.onactive = function onactive() {
    that.dispatchEvent({type: 'active'});
  };

  internalMS.oninactive = function oninactive() {
    that.dispatchEvent({type: 'inactive'});
  };

  internalMS.onaddtrack = function onaddtrack(internalMST) {
    var mst = new MediaStreamTrack(internalMST);

    that.dispatchEvent(new MediaStreamTrackEvent('addtrack', {track: mst}));
  };

  internalMS.onremovetrack = function onremovetrack(internalMST) {
    var mst = new MediaStreamTrack(internalMST);

    that.dispatchEvent(new MediaStreamTrackEvent('removetrack', {track: mst}));
  };

  // [ToDo] onended

  Object.defineProperties(this, {
    'id': {
      get: function getId() {
        return internalMS.id;
      }
    },
    'inactive': {
      get: function isInactive() {
        return internalMS.isinactive;
      }
    }
  });

  this._getMS = function _getMS() {
    return internalMS;
  };

  this.getAudioTracks = function getAudioTracks() {
    return internalMS.getAudioTracks();
  };

  this.getVideoTracks = function getVideoTracks() {
    return internalMS.getVideoTracks();
  };

  this.getTrackById = function getTrackById(id) {
    return internalMS.getTrackById(id);
  };

  this.addTrack = function addTrack(track) {
    internalMS.addTrack(track);
  };

  this.removeTrack = function removeTrack(track) {
    internalMS.removeTrack(track);
  };

  // FIXME: implement clone
  /*this.clone = function clone() {
    return internalMS.clone();
  };*/
}

module.exports = MediaStream;
