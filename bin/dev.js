'use strict';

// this script is only meant for dev mode of wrtc, and skipped in other cases.
if (process.env.npm_package_name &&
    process.env.npm_package_name !== 'wrtc' &&
    process.env.npm_config_dev !== 'true') {
    console.log('> wrtc dev mode skipped, but I can see it in your future.');
    return;
}

var path = require('path');
var npm = require('npm');

/*
 * why we install a dependency in this non standard way?
 * simple-peer has an optionalDependency back on this project.
 * to avoid circular dependency it was suggested to use the
 * --no-optional npm option when installing it, and for that
 * it can't appear in package.json (see issue #194).
 */
npmInstall('simple-peer', '4.2.2', {
    optional: false
});

function npmInstall(name, version, config) {
    var exactName = name + (version ? '@' + version : '');

    if (isInstalledExact(name, version)) {
        console.log('> Not need to reinstall', exactName);
        return;
    }

    console.log('> Installing', exactName, '...');
    npm.load(config, function(err) {
        exitOnError(err);
        npm.install(exactName, exitOnError);
    });
}

function isInstalledExact(name, version) {
    try {
        var pkg = require(path.join(name, 'package.json'));
        return pkg.version === version;
    } catch (err) {
        return false;
    }
}

function exitOnError(err) {
    if (err) {
        console.error('ERROR:', err.stack || err);
        process.exit(-1);
    }
}
