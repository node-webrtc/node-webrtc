'use strict';

function RTCError(code, message) {
  this.name = this.reasonName[Math.min(code, this.reasonName.length - 1)];
  this.message = typeof message === 'string' ? message : this.name;
}

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

module.exports = RTCError;
