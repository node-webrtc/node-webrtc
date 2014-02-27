function RTCDataChannelEvent(channel) {
  this.channel = channel;
}
RTCDataChannelEvent.prototype.type = 'datachannel';


module.exports = RTCDataChannelEvent;