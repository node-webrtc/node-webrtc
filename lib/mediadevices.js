var path = require('path');
var binary = require('node-pre-gyp');
var path = require('path');
//var binding_opts = require(path.resolve(path.join(__dirname, '../build/wrtc.json')));
var binding_path = binary.find(path.resolve(path.join(__dirname,'../package.json')));
var _webrtc = require(binding_path);

var EventTarget = require('./eventtarget');

function MediaDevices() {
  'use strict';
  var that = this
    , md = new _webrtc.MediaDevices()
    , localType = null
    , remoteType = null
    , queue = []
    , pending = null;

  EventTarget.call(this);

  function checkClosed() {
//    if(this._closed) {
//      throw new Error('Peer is closed');
//    }
  }

  function executeNext() {
    if(queue.length > 0) {
      var obj = queue.shift();

      md[obj.func].apply(md, obj.args);

      if(obj.wait)
      {
        pending = obj;
      } else {
        executeNext();
      }
    } else {
      pending = null;
    }
  }

  function queueOrRun(obj) {
    console.log('------------------- queued: ' + obj.func + ', pending: ' + pending);
    checkClosed();

    if(null === pending) {
      md[obj.func].apply(md, obj.args);

      if(obj.wait) {
        pending = obj;
      }
    } else {
      queue.push(obj);
    }
  }

  function runImmediately(obj) {
    checkClosed();

    return md[obj.func].apply(md, obj.args);
  }

  this.getUserMedia = function getUserMedia(successCallback, errorCallback){
    var stream = md.getUserMedia();
    if (stream === null) {
      return errorCallback(new Error("Couldn't create MediaStream"));
    }
    successCallback(stream);
  };
}

module.exports = MediaDevices;
