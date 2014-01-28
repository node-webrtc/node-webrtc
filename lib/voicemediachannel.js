var sys = require('sys');

var methodMapper = {
  setLocalRenderer: "setLocalRenderer",
  addSendStream: "addSendStream"
};

function callThisMethod(key) {
  return function innerCallThisMethod() {
    if(typeof this[key] !== "function") {
      throw new Error("Tried to call a method that didn't exist");
    }

    return this._vmc[key].apply(this._vmc, arguments);
  };
}

function privateVoiceMediaChannel(_internalVmc) {
  this._vmc = _internalVmc;
}

for(var key in methodMapper) {
  privateVoiceMediaChannel.prototype[key] = callThisMethod(key);
}

function callMethod() {
  arguments = Array.prototype.slice.call(arguments);
  var key = arguments.shift();

  if(typeof this[key] !== "function") {
    throw new Error("Tried to call a method that didn't exist");
  }

  return this[key].apply(this, arguments);
}

function VoiceMediaChannel(_internalVMC) {
  var vmc = new privateVoiceMediaChannel(_internalVMC);

  for(var key in methodMapper) {
    this[key] = callMethod.bind(vmc, key);
  }
}

module.exports = VoiceMediaChannel;

/* ex: set tabstop=2 softtabstop=2 shiftwidth=2 expandtab: */

