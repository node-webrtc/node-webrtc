function RTCDataChannelMessageEvent(message) {
  'use strict';
  this.data = message;
}
RTCDataChannelMessageEvent.prototype.type = 'message';

module.exports = RTCDataChannelMessageEvent;
