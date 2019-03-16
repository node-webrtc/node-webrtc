'use strict';

const EventEmitter = require('events');

class Connection extends EventEmitter {
  constructor(id) {
    super();
    this.id = id;
    this.state = 'open';
  }

  close() {
    this.state = 'closed';
    this.emit('closed');
  }

  toJSON() {
    return {
      id: this.id,
      state: this.state
    };
  }
}

module.exports = Connection;
