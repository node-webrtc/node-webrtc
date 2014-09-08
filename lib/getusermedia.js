var binary = require('node-pre-gyp');
var path = require('path');
var binding_path = binary.find(path.resolve(path.join(__dirname,'../package.json')));
var _webrtc = require(binding_path);

var MediaStream = require('./mediastream');

function getUserMedia(constraints, successCallback, errorCallback) {
  var stream = _webrtc.getUserMedia(constraints);
  if (stream === null) {
    return errorCallback(new Error("Couldn't create MediaStream"));
  }
  successCallback(new MediaStream(stream));
}

module.exports = getUserMedia;
