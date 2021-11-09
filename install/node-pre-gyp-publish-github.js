'use strict';

var path = require('path');
var fs = require('fs');
var cwd = process.cwd();

class NodePreGypGithub {
    constructor() {
        var ownerRepo, hostPrefix;

        this.package_json = JSON.parse(fs.readFileSync(path.join(cwd, 'package.json')));

        if (!this.package_json.repository || !this.package_json.repository.url) {
            throw new Error('Missing repository.url in package.json');
        }
        else {
            ownerRepo = this.package_json.repository.url.match(/https?:\/\/([^\/]+)\/(.*)(?=\.git)/i);
            if (ownerRepo) {
                this.host = 'api.' + ownerRepo[1];
                ownerRepo = ownerRepo[2].split('/');
                this.owner = ownerRepo[0];
                this.repo = ownerRepo[1];
            }
            else
                throw new Error('A correctly formatted GitHub repository.url was not found within package.json');
        }

        hostPrefix = 'https://' + this.host + '/' + this.owner + '/' + this.repo + '/releases/download/';
        if (!this.package_json.binary || 'object' !== typeof this.package_json.binary || 'string' !== typeof this.package_json.binary.host) {
            throw new Error('Missing binary.host in package.json');
        }
        else if (this.package_json.binary.host.replace('https://', 'https://api.').substr(0, hostPrefix.length) !== hostPrefix) {
            throw new Error('binary.host in package.json should begin with: "' + hostPrefix + '"');
        }
        const ocotkit = require('@octokit/rest');
        this.octokit = new ocotkit.Octokit({
            auth: this.authenticate_settings(),
            baseUrl: 'https://' + this.host,
            headers: {
                'user-agent': (this.package_json.name) ? this.package_json.name : 'node-pre-gyp-github'
            }
        });

    }
    authenticate_settings() {
        var token = process.env.NODE_PRE_GYP_GITHUB_TOKEN;
        if (!token)
            throw new Error('NODE_PRE_GYP_GITHUB_TOKEN environment variable not found');
        return token;
    }
    async createRelease(args) {
        var options = {
            'host': this.host,
            'owner': this.owner,
            'repo': this.repo,
            'tag_name': this.package_json.version,
            'target_commitish': 'develop',
            'name': 'v' + this.package_json.version,
            'body': this.package_json.name + ' ' + this.package_json.version,
            'draft': true,
            'prerelease': false
        };

        Object.keys(args).forEach(function (key) {
            if (args.hasOwnProperty(key) && options.hasOwnProperty(key)) {
                options[key] = args[key];
            }
        });
        console.log(this.owner, this.repo);
        return this.octokit.repos.createRelease(options);
    }
    uploadAssets() {
        var asset;
        console.log('Stage directory path: ' + path.join(this.stage_dir));
        fs.readdir(path.join(this.stage_dir), async (err, files) => {
            if (err)
                throw err;

            if (!files.length)
                throw new Error('No files found within the stage directory: ' + this.stage_dir);

            files.forEach(async (file) => {
                if (this.release && this.release.assets) {
                    asset = this.release.assets.filter(function (element, index, array) {
                        return element.name === file;
                    });
                    if (asset.length) {
                        throw new Error('Staged file ' + file + ' found but it already exists in release ' + this.release.tag_name + '. If you would like to replace it, you must first manually delete it within GitHub.');
                    }
                }
                console.log('Staged file ' + file + ' found. Proceeding to upload it.');

                const full = path.join(this.stage_dir, file);
                await this.octokit.repos.uploadReleaseAsset({
                    data: fs.readFileSync(full),
                    owner: this.owner,
                    release_id: this.release.id,
                    repo: this.repo,
                    name: file,
                });
                console.log('Staged file ' + file + ' saved to ' + this.owner + '/' + this.repo + ' release ' + this.release.tag_name + ' successfully.');
            });
        });
    }
    async publish(options) {
        options = (typeof options === 'undefined') ? {} : options;
        const data = await this.octokit.repos.listReleases({
            'owner': this.owner,
            'repo': this.repo
        });

        var release;
        // when remote_path is set expect files to be in stage_dir / remote_path after substitution
        if (this.package_json.binary.remote_path) {
            options.tag_name = this.package_json.binary.remote_path.replace(/\{version\}/g, this.package_json.version);
            this.stage_dir = path.join(this.stage_dir, options.tag_name);
        } else {
            // This is here for backwards compatibility for before binary.remote_path support was added in version 1.2.0.
            options.tag_name = this.package_json.version;
        }

        console.log(options);

        release = data.data.filter(function (element, index, array) {
            return element.tag_name === options.tag_name;
        });

        if (release.length === 0) {
            const release = await this.createRelease(options);
            this.release = release.data;
            console.log(this.release);
            if (this.release.draft) {
                console.log('Release ' + this.release.tag_name + ' not found, so a draft release was created. YOU MUST MANUALLY PUBLISH THIS DRAFT WITHIN GITHUB FOR IT TO BE ACCESSIBLE.');
            }
            else {
                console.log('Release ' + release.tag_name + ' not found, so a new release was created and published.');
            }
        }
        else {
            this.release = release[0];
        }

        this.uploadAssets();
    }
}
NodePreGypGithub.prototype.stage_dir = path.join(cwd, 'build', 'stage');
module.exports = NodePreGypGithub;