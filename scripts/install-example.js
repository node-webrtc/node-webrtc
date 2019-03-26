#!/usr/bin/env node
/* eslint no-console:0, no-process-env:0, no-process-exit:0 */
'use strict';

const { spawnSync } = require('child_process');
const { symlinkSync, writeFileSync } = require('fs');
const { join } = require('path');

const CWD = join(__dirname, '..');
const NODE_WEBRTC_EXAMPLES_URL = 'https://github.com/node-webrtc/node-webrtc-examples';
const NODE_WEBRTC_EXAMPLES_DIR = 'example';
const NODE_WEBRTC_EXAMPLES_REF = '445355403c943457200b5aba687fec45618c1c8a';

function gitClone(cwd, url, dir) {
  const description = 'git clone ' + url + ' ' + dir;
  console.log(description);
  let result = spawnSync('git', ['clone', url, dir], {
    cwd,
    shell: true,
    stdio: 'inherit'
  });
  if (result.status) {
    throw new Error(description + ' failed');
  }
}

function gitCheckout(cwd, dir, ref) {
  const description = 'git checkout ' + ref;
  console.log(description);
  let result = spawnSync('git', ['checkout', ref], {
    cwd: join(cwd, dir),
    shell: true,
    stdio: 'inherit'
  });
  if (result.status) {
    throw new Error(description + ' failed');
  }
}

function updatePackageJson(cwd, dir, fn) {
  console.log('Updating package.json');
  const packageJsonPath = join(cwd, dir, 'package.json');
  const packageJson = require(packageJsonPath);
  fn(packageJson);
  writeFileSync(packageJsonPath, JSON.stringify(packageJson, null, 2));
}

function npmInstall(cwd, dir) {
  const description = 'npm install';
  console.log(description);
  let result = spawnSync('npm', ['install'], {
    cwd: join(cwd, dir),
    shell: true,
    stdio: 'inherit'
  });
  if (result.status) {
    throw new Error(description + ' failed');
  }
}

function symlink(cwd, target, path) {
  const description = 'symlink ' + target + ' -> ' + path;
  console.log(description);
  process.chdir(cwd);
  symlinkSync(target, path, 'dir');
}

function deleteWrtcDependency(packageJson) {
  delete packageJson.dependencies.wrtc;
}

function main() {
  gitClone(CWD, NODE_WEBRTC_EXAMPLES_URL, NODE_WEBRTC_EXAMPLES_DIR);
  gitCheckout(CWD, NODE_WEBRTC_EXAMPLES_DIR, NODE_WEBRTC_EXAMPLES_REF);
  updatePackageJson(CWD, NODE_WEBRTC_EXAMPLES_DIR, deleteWrtcDependency);
  npmInstall(CWD, NODE_WEBRTC_EXAMPLES_DIR);
  symlink(join(CWD, NODE_WEBRTC_EXAMPLES_DIR, 'node_modules'), CWD, 'wrtc');
}

main();
