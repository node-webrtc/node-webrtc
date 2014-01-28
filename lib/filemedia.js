var _webrtc = require('bindings')('webrtc.node');
var VoiceMediaChannel = require('./voicemediachannel');
var MediaStream = require('./mediastream');

var sys = require('sys');

var fileMapper = {
  voiceInput: "setVoiceInputFilename",
  voiceOutput: "setVoiceOutputFilename",
  videoInput: "setVideoInputFilename",
  videoOutput: "setVideoOutputFilename"
};

function privateFileMedia(args) {
  args = args || {};
  var that = this;
  this._fm = new _webrtc.FileMedia();

  for(var key in fileMapper) {
    this[key] = callMethod.bind(this, key);
    if(args[key]) {
      if(typeof args[key] !== "string") {
        throw new Error(key + " must be a path to a file (string)");
      }
      this._fm[fileMapper[key]](args[key]);
    }
  }
}

privateFileMedia.prototype.createChannel = function() {
  var _internalVMC = this._fm.createChannel();
  if(_internalVMC === null) {
    throw new Error("One of your files didn't exist!");
  }

  return new VoiceMediaChannel(_internalVMC);
};

privateFileMedia.prototype.createStream = function() {
  var _stream = this._fm.createStream();
  if(_stream === null) {
    throw new Error("Couldn't create a stream!");
  }

  return new MediaStream(_stream);
};

function callMethod() {
  var key = args.shift();

  if(typeof this[key] !== "function") {
    throw new Error("Tried to call a method that didn't exist");
  }

  this[key].apply(this, arguments);
}

function FileMedia(args) {
  var fm = new privateFileMedia(args);

  for(var key in fileMapper) {
    this[key] = callMethod.bind(fm, key);
  }

  this.createChannel = function(){
    return fm.createChannel.apply(fm, arguments);
  };

  this.createStream = function(){
    return fm.createStream.apply(fm, arguments);
  };
}

module.exports = FileMedia;

/* ex: set tabstop=2 softtabstop=2 shiftwidth=2 expandtab: */
