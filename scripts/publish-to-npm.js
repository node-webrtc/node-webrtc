#!/usr/bin/env node
'use strict';

const { spawnSync } = require('child_process');
const { readFileSync, writeFileSync } = require('fs');
const { join } = require('path');
const copy = require('recursive-copy');
const temp = require('temp');

const rootPackageJson = require('../package.json');

temp.track();

const githubUrl = 'https://github.com/node-webrtc/node-webrtc/blob/v' + rootPackageJson.version;

const paths = [
  'lib',
  'scripts/download-prebuilt.js',
  'LICENSE.md',
  'README.md',
  'THIRD_PARTY_LICENSES.md'
];

const jsonFields = [
  'name',
  'description',
  'keywords',
  'version',
  'author',
  'homepage',
  'bugs',
  'license',
  'repository',
  'main',
  'browser',
  'binary',
  'engines',
  'optionalDependencies',
  'bundledDependencies'
];

const relativeLinks = [
  'docs/build-from-source.md',
  'docs/nonstandard-apis.md'
];

function mkTmpDir(dir) {
  return new Promise((resolve, reject) => {
    temp.mkdir(dir, (error, dir) => {
      if (error) {
        reject(error);
      } else {
        resolve(dir);
      }
    });
  });
}

async function main() {
  const { name } = rootPackageJson;
  const tmpDir = await mkTmpDir(name);

  await Promise.all(paths.map(async path => {
    const src = join(__dirname, '..', path);
    const dst = join(tmpDir, path);
    await copy(src, dst);
  }));

  const packageJson = require('../npm/package.json');
  jsonFields.forEach(jsonField => {
    packageJson[jsonField] = rootPackageJson[jsonField];
  });
  writeFileSync(
    join(tmpDir, 'package.json'),
    JSON.stringify(packageJson, null, 2)
  );

  const readme = relativeLinks.reduce((readme, relativeLink) => {
    const regexp = new RegExp('(' + relativeLink.replace(/\./g, '\\.') + ')', 'g');
    return readme.replace(regexp, githubUrl + '/$1');
  }, readFileSync(join(__dirname, '..', 'README.md')).toString());

  writeFileSync(
    join(tmpDir, 'README.md'),
    readme
  );

  const { status } = spawnSync('npm', ['publish'], {
    shell: true,
    stdio: 'inherit',
    cwd: tmpDir
  });
  if (status) {
    throw new Error('npm publish failed');
  }
}

main();
