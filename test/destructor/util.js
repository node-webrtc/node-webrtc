/* globals gc */
'use strict';

const { createHook } = require('async_hooks');

/**
 * @interface Deferred<T>
 * @property {function(result: T): void} resolve
 * @property {function(reason: any): void} reject
 * @property {Promise<T>} promise
 */

/**
 * Create a {@link Deferred}.
 * @returns {Deferred<T>}
 */
function createDeferred() {
  const deferred = {};
  deferred.promise = new Promise((resolve, reject) => Object.assign(deferred, {
    resolve,
    reject
  }));
  return deferred;
}

const typesToIgnore = new Set([
  'FSEVENTWRAP', 'FSREQCALLBACK', 'GETADDRINFOREQWRAP', 'GETNAMEINFOREQWRAP', 'HTTPINCOMINGMESSAGE',
  'HTTPCLIENTREQUEST', 'JSSTREAM', 'PIPECONNECTWRAP', 'PIPEWRAP', 'PROCESSWRAP', 'QUERYWRAP',
  'SHUTDOWNWRAP', 'SIGNALWRAP', 'STATWATCHER', 'TCPCONNECTWRAP', 'TCPSERVERWRAP', 'TCPWRAP',
  'TTYWRAP', 'UDPSENDWRAP', 'UDPWRAP', 'WRITEWRAP', 'ZLIB', 'SSLCONNECTION', 'PBKDF2REQUEST',
  'RANDOMBYTESREQUEST', 'TLSWRAP', 'Microtask', 'Timeout', 'Immediate', 'TickObject', 'PROMISE'
]);

function trackDestructors() {
  const asyncIds = new WeakMap();
  const destructorDeferreds = new Map();

  function createDestructorDeferred(resource, asyncId) {
    const destructorDeferred = createDeferred();
    asyncIds.set(resource, asyncId);
    destructorDeferreds.set(asyncId, destructorDeferred);
    return destructorDeferred;
  }

  function maybeResolveDestructorDeferred(asyncId) {
    const destructorDeferred = destructorDeferreds.get(asyncId);
    if (destructorDeferred) {
      destructorDeferreds.delete(asyncId);
      destructorDeferred.resolve();
      return true;
    }
    return false;
  }

  function getDestructorPromise(resource) {
    const asyncId = asyncIds.get(resource);
    if (!asyncId) {
      return Promise.reject(new Error('Unknown resource'));
    }
    const destructorDeferred = destructorDeferreds.get(asyncId);
    return destructorDeferred
      ? destructorDeferred.promise
      : Promise.reject(new Error('Unknown asyncId'));
  }

  const interval = setInterval(gc);

  const asyncHook = createHook({
    init(asyncId, type, triggerAsyncId, resource) {
      if (typesToIgnore.has(type)) {
        return;
      }
      createDestructorDeferred(resource, asyncId);
    },
    destroy(asyncId) {
      maybeResolveDestructorDeferred(asyncId);
    }
  });

  asyncHook.enable();

  return {
    destructor: getDestructorPromise,
    stop() {
      clearInterval(interval);
      asyncHook.disable();
    }
  };
}

module.exports = {
  createDeferred,
  trackDestructors
};
