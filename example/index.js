'use strict';

const bodyParser = require('body-parser');
const browserify = require('browserify-middleware');
const express = require('express');
const { readdirSync, statSync } = require('fs');
const { join } = require('path');

const { mount } = require('./lib/server/rest/connectionsapi');
const WebRtcConnectionManager = require('./lib/server/connections/webrtcconnectionmanager');

const app = express();

app.use(bodyParser.json());

const examplesDirectory = join(__dirname, 'examples');

const examples = readdirSync(examplesDirectory).filter(path =>
  statSync(join(examplesDirectory, path)).isDirectory());

function setupExample(example) {
  const path = join(examplesDirectory, example);
  const clientPath = join(path, 'client.js');
  const serverPath = join(path, 'server.js');

  app.use(`/${example}/index.js`, browserify(clientPath));
  app.get(`/${example}/index.html`, (req, res) => {
    res.sendFile(join(__dirname, 'html', 'index.html'));
  });

  const options = require(serverPath);
  const connectionManager = WebRtcConnectionManager.create(options);
  mount(app, connectionManager, `/${example}`);

  return connectionManager;
}

app.get('/', (req, res) => res.redirect(`${examples[0]}/index.html`));

const connectionManagers = examples.reduce((connectionManagers, example) => {
  const connectionManager = setupExample(example);
  return connectionManagers.set(example, connectionManager);
}, new Map());

const server = app.listen(3000, () => {
  const address = server.address();
  console.log(`http://localhost:${address.port}\n`);

  server.once('close', () => {
    connectionManagers.forEach(connectionManager => connectionManager.close());
  });
});
