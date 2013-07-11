var addon = require('./build/Release/addon');
var test = new addon.Test();

console.log(null === test.oncomplete);
console.log(typeof test.RunCallback);

test.oncomplete = function() {
  console.log('oncomplete!');
};

test.RunCallback();