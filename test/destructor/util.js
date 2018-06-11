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

/**
 * Check that an AsyncResource, T, is destroyed.
 * @param {string} type
 * @param {function(): (T|Promise<T>)} create
 * @param {function(created: T): (void|Promise<void>)} destroy
 */
async function checkDestructor(type, create, destroy) {
  const initDeferred = createDeferred();
  const destroyDeferred = createDeferred();

  let asyncId;

  const asyncHook = createHook({
    init(_asyncId, _type) {
      if (_type !== type || typeof asyncId === 'number') {
        return;
      }
      asyncId = _asyncId;
      initDeferred.resolve();
    },
    destroy(_asyncId) {
      if (_asyncId !== asyncId) {
        return;
      }
      destroyDeferred.resolve();
    },
  });

  asyncHook.enable();

  const interval = setInterval(() => gc());

  // NOTE(mroberts): We need to do this in a closure so that the AsyncResource
  // can be garbage collected.
  await (async () => {
    const created = await create();
    await initDeferred.promise;
    await destroy(created);
  })();

  await destroyDeferred.promise;
  clearInterval(interval);

  asyncHook.disable();
}

module.exports = checkDestructor;
