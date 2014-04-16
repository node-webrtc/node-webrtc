/**
 * @author mrdoob / http://mrdoob.com/
 * @author Jesús Leganés Combarro "Piranna" <piranna@gmail.com>
 */

var EventTarget = function()
{
	var listeners = {};

	this.addEventListener = function(type, listener)
	{
		if(listeners[type] === undefined)
			listeners[type] = [];

		if(listeners[type].indexOf(listener) === -1)
			listeners[type].push(listener);
	};

	this.dispatchEvent = function(event)
	{
		process.nextTick(function() {
			var listenerArray = (listeners[event.type] || []);

			var dummyListener = this['on' + event.type];
			if(typeof dummyListener == 'function')
				listenerArray = listenerArray.concat(dummyListener);

			for(var i=0, l=listenerArray.length; i<l; i++)
				listenerArray[i].call(this, event);
		}.bind(this));
	};

	this.removeEventListener = function(type, listener)
	{
		var index = listeners[type].indexOf(listener);

		if(index !== -1)
			listeners[type].splice(index, 1);
	};
};

module.exports = EventTarget;
