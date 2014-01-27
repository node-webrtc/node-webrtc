var _webrtc = require('bindings')('webrtc.node');
var VoiceMediaChannel = require('./voicemediachannel');

var sys = require('sys');

var mapper = {
  voiceInput: "setVoiceInputFilename",
  voiceOutput: "setVoiceOutputFilename",
  videoInput: "setVideoInputFilename",
  videoOutput: "setVideoOutputFilename"
};


function privateFileMedia(args) {
  args = args || {};
  var that = this;
  this._fm = new _webrtc.FileMedia();

  for(var key in mapper) {
    this[key] = callMethod.bind(this, key);
    if(args[key]) {
      if(typeof args[key] !== "string") {
        throw new Error(key + " must be a path to a file (string)");
      }
      this._fm[mapper[key]](args[key]);
    }
  }
}

privateFileMedia.prototype.createChannel = function() {
  var _internalVMC = this._fm.createChannel();
  return new VoiceMediaChannel(_internalVMC);
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

  for(var key in mapper) {
    this[key] = callMethod.bind(fm, key);
  }

  this.createChannel = function(){
    var channel = fm.createChannel.apply(fm, arguments);
    return channel;
  };
}

module.exports = FileMedia;

/* ex: set tabstop=2 softtabstop=2 shiftwidth=2 expandtab: */
