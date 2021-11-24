#!/usr/bin/env node

const fs = require('fs'), path = require('path');

fs.rmSync('./python2', {force: true})

const target = 'python2';
for (let pe of process.env['PATH']?.split(':') ?? []) {
    var at = path.join(pe, target);
    // life is too short to check the exec mode bit
    try { fs.statSync(at).isFile() && fs.symlinkSync(at, './python2'); break; }
    catch { }
}
