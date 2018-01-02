'use strict';

function RTCDataChannelMessageEvent(message) {
  this.data = message;
}

RTCDataChannelMessageEvent.prototype.type = 'message';

module.exports = RTCDataChannelMessageEvent;
