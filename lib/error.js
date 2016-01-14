function RTCError(code, message) {
  'use strict';
  this.name = this.reasonName[Math.min(code, this.reasonName.length - 1)];
  this.message = (typeof message === 'string')? message : this.name;
}

module.exports = RTCError;

RTCError.prototype.reasonName = [
  // These strings must match those defined in the WebRTC spec.
  'NO_ERROR', // Should never happen -- only used for testing
  'INVALID_CONSTRAINTS_TYPE',
  'INVALID_CANDIDATE_TYPE',
  'INVALID_STATE',
  'INVALID_SESSION_DESCRIPTION',
  'INCOMPATIBLE_SESSION_DESCRIPTION',
  'INCOMPATIBLE_CONSTRAINTS',
  'INTERNAL_ERROR'
];
