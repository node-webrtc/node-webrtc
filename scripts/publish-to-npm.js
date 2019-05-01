#!/usr/bin/env node
'use strict';

const { spawnSync } = require('child_process');
const { writeFileSync } = require('fs');
const { join } = require('path');
const copy = require('recursive-copy');
const temp = require('temp');

const rootPackageJson = require('../package.json');

temp.track();

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
