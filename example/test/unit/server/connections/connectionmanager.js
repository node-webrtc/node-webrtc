'use strict';

const tape = require('tape');

const Connection = require('../../../../lib/server/connections/connection');
const ConnectionManager = require('../../../../lib/server/connections/connectionmanager');

tape('ConnectionManager', t => {
  t.test('constructor', t => {
    const connectionManager = new ConnectionManager();
    t.ok(connectionManager instanceof ConnectionManager, 'returns an instance of ConnectionManager');
    t.end();
  });

  t.test('.createConnection()', t => {
    const ids = [1, 1, 1, 2];
    function generateId() {
      return ids.shift();
    }

    const connectionManager = new ConnectionManager({ generateId });

    const connection1 = connectionManager.createConnection();
    t.ok(connection1 instanceof Connection, 'returns an instance of Connection');
    t.equal(connection1.id, 1, 'it sets the Connection\'s ID to the generated ID');

    const connection2 = connectionManager.createConnection();
    t.ok(connection2.id, 2, 'it calls generateId() until a unique ID is returned');

    t.end();
  });

  t.test('.getConnection(id)', t => {
    const connectionManager = new ConnectionManager();
    const connection = connectionManager.createConnection();
    t.equal(connectionManager.getConnection('foo'), null, '.getConnection(id) with an unknown id returns null');
    t.equal(connectionManager.getConnection(connection.id), connection, '.getConnection(id) with a known id returns a Connection');
    connection.close();
    t.equal(connectionManager.getConnection(connection.id), null, '.getConnection(id) returns null once the Connection with that ID closes');
    t.end();
  });

  t.test('.getConnections()', t => {
    const connectionManager = new ConnectionManager();
    t.deepEqual(connectionManager.getConnections(), [], '.getConnections() initially returns an empty array');
    const connection1 = connectionManager.createConnection();
    t.deepEqual(connectionManager.getConnections(), [connection1], '.getConnections() returns the expected result when 1 Connection is open');
    const connection2 = connectionManager.createConnection();
    t.deepEqual(connectionManager.getConnections(), [connection1, connection2], '.getConnections() returns the expected result when 2 Connections are open');
    connection1.close();
    connection2.close();
    t.deepEqual(connectionManager.getConnections(), [], '.getConnections() returns an empty array when all Connections are closed');
    t.end();
  });

  t.end();
});
