'use strict';

const ConnectionManager = require('./connectionmanager');
const WebRtcConnection = require('./webrtcconnection');

class WebRtcConnectionManager {
  constructor(options = {}) {
    options = {
      Connection: WebRtcConnection,
      ...options
    };

    const connectionManager = new ConnectionManager(options);

    this.createConnection = async () => {
      const connection = connectionManager.createConnection();
      await connection.doOffer();
      return connection;
    };

    this.getConnection = id => {
      return connectionManager.getConnection(id);
    };

    this.getConnections = () => {
      return connectionManager.getConnections();
    };
  }

  toJSON() {
    return this.getConnections().map(connection => connection.toJSON());
  }
}

WebRtcConnectionManager.create = function create(options) {
  return new WebRtcConnectionManager({
    Connection: function(id) {
      return new WebRtcConnection(id, options);
    }
  });
};

module.exports = WebRtcConnectionManager;
