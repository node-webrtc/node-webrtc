var EventTarget = require('./eventtarget');

var RTCMediaStreamMessageEvent = require('./mediastreammessageevent');

function RTCMediaStream(internalMS) {
  'use strict';
  var that = this;

  EventTarget.call(this);

  that._internalMS = internalMS;
  internalMS.onerror = function onerror() {
    that.dispatchEvent({type: 'error'});
  };

  internalMS.onmessage = function onmessage(data) {
    var evt = new RTCMediaStreamMessageEvent(data);
    that.dispatchEvent(evt);
    that._onmessage && that._onmessage(evt);
  };

  that.getVideoTracks = function() {
    return internalMS.getVideoTracks();
  }

  Object.defineProperties(this, {
    'id': {
        get: function getID() {
            return internalMS.id;
        }
    },
    'active': {
        get: function() {
          return internalMS.active;
        }
    },
    'ended': {
        get: function() {
          return internalMS.readyState === 'closed';
        }
    },
    'onmessage': {
        set: function(onMessage) {
            that._onmessage = onMessage;
        }
    }
  });

  this.close = function close() {
    internalMS.close();
  };
}

RTCMediaStream.prototype.RTCMediaStates = [
  'connecting',
  'open',
  'closing',
  'closed'
];

module.exports = RTCMediaStream;
