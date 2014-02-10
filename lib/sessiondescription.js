function RTCSessionDescription(dict) {
  this.type = dict.type;
  this.sdp = dict.sdp;
};

module.exports = RTCSessionDescription;