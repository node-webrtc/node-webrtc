var module = require('./node-pre-gyp-publish-github');
var program = require('commander');
var package_json = require('../package.json');

program
	.command('publish [options]')
	.description('publishes the contents of .\\build\\stage\\{version} to the current version\'s GitHub release')
	.option("-r, --release", "publish immediately, do not create draft")
	.option("-s, --silent", "turns verbose messages off")
	.action(function(cmd, options){
		var opts = {},
			x = new module();
		x.package_json.version = package_json.version;  // in case it is being run from a subdir, e.g. `phase-1`
		opts.draft = options.release ? false : true;
		opts.verbose = options.silent ? false : true;
		x.publish(opts);
	});

program
	.command('help','',{isDefault: true, noHelp: true})
	.action(function() {
		console.log();
		console.log('Usage: node-pre-gyp-github publish');
		console.log();
		console.log('publishes the contents of .\\build\\stage\\{version} to the current version\'s GitHub release');
	});

program.parse(process.argv);

if (!program.args.length) {
	program.help();
}
