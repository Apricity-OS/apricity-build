(function() {
  var Command, View, config, optimist, request, semver, tree, _,
    __hasProp = {}.hasOwnProperty,
    __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

  _ = require('underscore-plus');

  optimist = require('optimist');

  semver = require('npm/node_modules/semver');

  Command = require('./command');

  config = require('./apm');

  request = require('./request');

  tree = require('./tree');

  module.exports = View = (function(_super) {
    __extends(View, _super);

    function View() {
      return View.__super__.constructor.apply(this, arguments);
    }

    View.commandNames = ['view', 'show'];

    View.prototype.parseOptions = function(argv) {
      var options;
      options = optimist(argv);
      options.usage("\nUsage: apm view <package_name>\n\nView information about a package/theme in the atom.io registry.");
      options.alias('h', 'help').describe('help', 'Print this usage message');
      options.boolean('json').describe('json', 'Output featured packages as JSON array');
      return options.string('compatible').describe('compatible', 'Show the latest version compatible with this Atom version');
    };

    View.prototype.loadInstalledAtomVersion = function(options, callback) {
      return process.nextTick((function(_this) {
        return function() {
          var installedAtomVersion, version;
          if (options.argv.compatible) {
            version = _this.normalizeVersion(options.argv.compatible);
            if (semver.valid(version)) {
              installedAtomVersion = version;
            }
          }
          return callback(installedAtomVersion);
        };
      })(this));
    };

    View.prototype.getLatestCompatibleVersion = function(pack, options, callback) {
      return this.loadInstalledAtomVersion(options, (function(_this) {
        return function(installedAtomVersion) {
          var engine, latestVersion, metadata, version, _ref, _ref1, _ref2, _ref3;
          if (!installedAtomVersion) {
            return callback(pack.releases.latest);
          }
          latestVersion = null;
          _ref1 = (_ref = pack.versions) != null ? _ref : {};
          for (version in _ref1) {
            metadata = _ref1[version];
            if (!semver.valid(version)) {
              continue;
            }
            if (!metadata) {
              continue;
            }
            engine = (_ref2 = (_ref3 = metadata.engines) != null ? _ref3.atom : void 0) != null ? _ref2 : '*';
            if (!semver.validRange(engine)) {
              continue;
            }
            if (!semver.satisfies(installedAtomVersion, engine)) {
              continue;
            }
            if (latestVersion == null) {
              latestVersion = version;
            }
            if (semver.gt(version, latestVersion)) {
              latestVersion = version;
            }
          }
          return callback(latestVersion);
        };
      })(this));
    };

    View.prototype.getRepository = function(pack) {
      var repository, _ref, _ref1;
      if (repository = (_ref = (_ref1 = pack.repository) != null ? _ref1.url : void 0) != null ? _ref : pack.repository) {
        return repository.replace(/\.git$/, '');
      }
    };

    View.prototype.getPackage = function(packageName, options, callback) {
      var requestSettings;
      requestSettings = {
        url: "" + (config.getAtomPackagesUrl()) + "/" + packageName,
        json: true
      };
      return request.get(requestSettings, (function(_this) {
        return function(error, response, body) {
          var message, _ref, _ref1;
          if (body == null) {
            body = {};
          }
          if (error != null) {
            return callback(error);
          } else if (response.statusCode === 200) {
            return _this.getLatestCompatibleVersion(body, options, function(version) {
              var downloads, metadata, name, pack, readme, stargazers_count, _ref, _ref1;
              name = body.name, readme = body.readme, downloads = body.downloads, stargazers_count = body.stargazers_count;
              metadata = (_ref = (_ref1 = body.versions) != null ? _ref1[version] : void 0) != null ? _ref : {
                name: name
              };
              pack = _.extend({}, metadata, {
                readme: readme,
                downloads: downloads,
                stargazers_count: stargazers_count
              });
              return callback(null, pack);
            });
          } else {
            message = (_ref = (_ref1 = body.message) != null ? _ref1 : body.error) != null ? _ref : body;
            return callback("Requesting package failed: " + message);
          }
        };
      })(this));
    };

    View.prototype.run = function(options) {
      var callback, packageName;
      callback = options.callback;
      options = this.parseOptions(options.commandArgs);
      packageName = options.argv._[0];
      if (!packageName) {
        callback("Missing required package name");
        return;
      }
      return this.getPackage(packageName, options, (function(_this) {
        return function(error, pack) {
          var items, repository;
          if (error != null) {
            callback(error);
            return;
          }
          if (options.argv.json) {
            console.log(JSON.stringify(pack, null, 2));
          } else {
            console.log("" + pack.name.cyan);
            items = [];
            if (pack.version) {
              items.push(pack.version.yellow);
            }
            if (repository = _this.getRepository(pack)) {
              items.push(repository.underline);
            }
            if (pack.description) {
              items.push(pack.description.replace(/\s+/g, ' '));
            }
            if (pack.downloads >= 0) {
              items.push(_.pluralize(pack.downloads, 'download'));
            }
            if (pack.stargazers_count >= 0) {
              items.push(_.pluralize(pack.stargazers_count, 'star'));
            }
            tree(items);
            console.log();
            console.log("Run `apm install " + pack.name + "` to install this package.");
            console.log();
          }
          return callback();
        };
      })(this));
    };

    return View;

  })(Command);

}).call(this);
