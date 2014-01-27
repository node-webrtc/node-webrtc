var sys = require('sys');

var mapper = {
  setLocalRenderer: "setLocalRenderer"
};


function privateVoiceMediaChannel(_internalVmc) {
  this._vmc = _internalVmc;
}

for(var key in mapper) {
  privateVoiceMediaChannel.prototype[key] = function() {
    callMethod.apply(this._vmc, arguments);
  };
}

function callMethod() {
  var key = args.shift();

  if(typeof this[key] !== "function") {
    throw new Error("Tried to call a method that didn't exist");
  }

  this[key].apply(this, arguments);
}

function VoiceMediaChannel(_internalVMC) {
  var vmc = new privateVoiceMediaChannel(_internalVMC);

  for(var key in mapper) {
    this[key] = callMethod.bind(vmc, key);
  }
}

module.exports = VoiceMediaChannel;

/* ex: set tabstop=2 softtabstop=2 shiftwidth=2 expandtab: */

