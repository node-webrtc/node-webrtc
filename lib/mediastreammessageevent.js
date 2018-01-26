function RTCMediaStreamMessageEvent(message) {
  'use strict';
  this.data = message;
}
RTCMediaStreamMessageEvent.prototype.type = 'message';

module.exports = RTCMediaStreamMessageEvent;
