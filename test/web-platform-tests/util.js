'use strict';

/**
 * Is this script currently running within a browser context?
 * Note: also returns true within a Web Worker context
 * @returns {boolean}
 */
function inBrowserContext() {
  /* globals window */
  return (typeof window === 'object' && window === window.self) || inWebWorkerContext();
}

function inWebWorkerContext() {
  /* globals WorkerGlobalScope, self */
  return typeof WorkerGlobalScope !== 'undefined' && self instanceof WorkerGlobalScope;
}

/**
 * Create a new Promise and call the given function with a resolver function which can be passed as a node
 * style callback.
 *
 * Node style callbacks expect their first argument to be an Error object or null. The returned promise will
 * be rejected if this first argument is set. Otherwise the promise will be resolved with the second argument of
 * the callback, or with an array of arguments if there are more arguments.
 *
 * @example nodeResolverPromise(nodeResolver => fs.readFile('foo.png', nodeResolver)).then(content => {});
 * @param {Function} fn
 * @returns {Promise}
 */
function nodeResolverPromise(fn) {
  return new Promise((resolve, reject) => {
    fn(function(error, result) {
      if (error) {
        reject(error);
      } else if (arguments.length > 2) {
        // pass all the arguments as an array,
        // skipping the error param
        const arrayResult = new Array(arguments.length - 1);
        for (let i = 1; i < arguments.length; ++i) {
          arrayResult[i - 1] = arguments[i];
        }
        resolve(arrayResult);
      } else {
        resolve(result);
      }
    });
  });
}

exports.inBrowserContext = inBrowserContext;
exports.inWebWorkerContext = inWebWorkerContext;
exports.nodeResolverPromise = nodeResolverPromise;
