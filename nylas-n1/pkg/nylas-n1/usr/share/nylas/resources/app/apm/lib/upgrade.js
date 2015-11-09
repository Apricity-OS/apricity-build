(function() {
  var Command, Install, Packages, Upgrade, async, config, fs, optimist, path, read, request, semver, tree, _,
    __hasProp = {}.hasOwnProperty,
    __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

  path = require('path');

  _ = require('underscore-plus');

  async = require('async');

  optimist = require('optimist');

  read = require('read');

  semver = require('npm/node_modules/semver');

  Command = require('./command');

  config = require('./apm');

  fs = require('./fs');

  Install = require('./install');

  Packages = require('./packages');

  request = require('./request');

  tree = require('./tree');

  module.exports = Upgrade = (function(_super) {
    __extends(Upgrade, _super);

    Upgrade.commandNames = ['upgrade', 'outdated', 'update'];

    function Upgrade() {
      this.atomDirectory = config.getAtomDirectory();
      this.atomPackagesDirectory = path.join(this.atomDirectory, 'packages');
    }

    Upgrade.prototype.parseOptions = function(argv) {
      var options;
      options = optimist(argv);
      options.usage("\nUsage: apm upgrade\n       apm upgrade --list\n       apm upgrade [<package_name>...]\n\nUpgrade out of date packages installed to ~/.atom/packages\n\nThis command lists the out of date packages and then prompts to install\navailable updates.");
      options.alias('c', 'confirm').boolean('confirm')["default"]('confirm', true).describe('confirm', 'Confirm before installing updates');
      options.alias('h', 'help').describe('help', 'Print this usage message');
      options.alias('l', 'list').boolean('list').describe('list', 'List but don\'t install the outdated packages');
      options.boolean('json').describe('json', 'Output outdated packages as a JSON array');
      options.string('compatible').describe('compatible', 'Only list packages/themes compatible with this Atom version');
      return options.boolean('verbose')["default"]('verbose', false).describe('verbose', 'Show verbose debug information');
    };

    Upgrade.prototype.getInstalledPackages = function(options) {
      var name, pack, packageNames, packages, _i, _len, _ref;
      packages = [];
      _ref = fs.list(this.atomPackagesDirectory);
      for (_i = 0, _len = _ref.length; _i < _len; _i++) {
        name = _ref[_i];
        if (pack = this.getIntalledPackage(name)) {
          packages.push(pack);
        }
      }
      packageNames = this.packageNamesFromArgv(options.argv);
      if (packageNames.length > 0) {
        packages = packages.filter(function(_arg) {
          var name;
          name = _arg.name;
          return packageNames.indexOf(name) !== -1;
        });
      }
      return packages;
    };

    Upgrade.prototype.getIntalledPackage = function(name) {
      var metadata, packageDirectory;
      packageDirectory = path.join(this.atomPackagesDirectory, name);
      if (fs.isSymbolicLinkSync(packageDirectory)) {
        return;
      }
      try {
        metadata = JSON.parse(fs.readFileSync(path.join(packageDirectory, 'package.json')));
        if ((metadata != null ? metadata.name : void 0) && (metadata != null ? metadata.version : void 0)) {
          return metadata;
        }
      } catch (_error) {}
    };

    Upgrade.prototype.loadInstalledAtomVersion = function(options, callback) {
      if (options.argv.compatible) {
        return process.nextTick((function(_this) {
          return function() {
            var version;
            version = _this.normalizeVersion(options.argv.compatible);
            if (semver.valid(version)) {
              _this.installedAtomVersion = version;
            }
            return callback();
          };
        })(this));
      } else {
        return config.getResourcePath((function(_this) {
          return function(resourcePath) {
            var version, _ref;
            try {
              version = ((_ref = require(path.join(resourcePath, 'package.json'))) != null ? _ref : {}).version;
              version = _this.normalizeVersion(version);
              if (semver.valid(version)) {
                _this.installedAtomVersion = version;
              }
            } catch (_error) {}
            return callback();
          };
        })(this));
      }
    };

    Upgrade.prototype.getLatestVersion = function(pack, callback) {
      var requestSettings;
      requestSettings = {
        url: "" + (config.getAtomPackagesUrl()) + "/" + pack.name,
        json: true
      };
      return request.get(requestSettings, (function(_this) {
        return function(error, response, body) {
          var atomVersion, engine, latestVersion, message, metadata, version, _ref, _ref1, _ref2, _ref3, _ref4, _ref5;
          if (body == null) {
            body = {};
          }
          if (error != null) {
            return callback("Request for package information failed: " + error.message);
          } else if (response.statusCode === 404) {
            return callback();
          } else if (response.statusCode !== 200) {
            message = (_ref = (_ref1 = body.message) != null ? _ref1 : body.error) != null ? _ref : body;
            return callback("Request for package information failed: " + message);
          } else {
            atomVersion = _this.installedAtomVersion;
            latestVersion = pack.version;
            _ref3 = (_ref2 = body.versions) != null ? _ref2 : {};
            for (version in _ref3) {
              metadata = _ref3[version];
              if (!semver.valid(version)) {
                continue;
              }
              if (!metadata) {
                continue;
              }
              engine = (_ref4 = (_ref5 = metadata.engines) != null ? _ref5.atom : void 0) != null ? _ref4 : '*';
              if (!semver.validRange(engine)) {
                continue;
              }
              if (!semver.satisfies(atomVersion, engine)) {
                continue;
              }
              if (semver.gt(version, latestVersion)) {
                latestVersion = version;
              }
            }
            if (latestVersion !== pack.version && _this.hasRepo(pack)) {
              return callback(null, {
                pack: pack,
                latestVersion: latestVersion
              });
            } else {
              return callback();
            }
          }
        };
      })(this));
    };

    Upgrade.prototype.hasRepo = function(pack) {
      return Packages.getRepository(pack) != null;
    };

    Upgrade.prototype.getAvailableUpdates = function(packages, callback) {
      return async.map(packages, this.getLatestVersion.bind(this), (function(_this) {
        return function(error, updates) {
          if (error != null) {
            return callback(error);
          }
          updates = _.compact(updates);
          updates.sort(function(updateA, updateB) {
            return updateA.pack.name.localeCompare(updateB.pack.name);
          });
          return callback(null, updates);
        };
      })(this));
    };

    Upgrade.prototype.promptForConfirmation = function(callback) {
      return read({
        prompt: 'Would you like to install these updates? (yes)',
        edit: true
      }, function(error, answer) {
        answer = answer ? answer.trim().toLowerCase() : 'yes';
        return callback(error, answer === 'y' || answer === 'yes');
      });
    };

    Upgrade.prototype.installUpdates = function(updates, callback) {
      var installCommands, latestVersion, pack, verbose, _fn, _i, _len, _ref;
      installCommands = [];
      verbose = this.verbose;
      _fn = function(pack, latestVersion) {
        return installCommands.push(function(callback) {
          var commandArgs;
          commandArgs = ["" + pack.name + "@" + latestVersion];
          if (verbose) {
            commandArgs.unshift('--verbose');
          }
          return new Install().run({
            callback: callback,
            commandArgs: commandArgs
          });
        });
      };
      for (_i = 0, _len = updates.length; _i < _len; _i++) {
        _ref = updates[_i], pack = _ref.pack, latestVersion = _ref.latestVersion;
        _fn(pack, latestVersion);
      }
      return async.waterfall(installCommands, callback);
    };

    Upgrade.prototype.run = function(options) {
      var callback, command;
      callback = options.callback, command = options.command;
      options = this.parseOptions(options.commandArgs);
      options.command = command;
      this.verbose = options.argv.verbose;
      if (this.verbose) {
        request.debug(true);
        process.env.NODE_DEBUG = 'request';
      }
      return this.loadInstalledAtomVersion(options, (function(_this) {
        return function() {
          if (_this.installedAtomVersion) {
            return _this.upgradePackages(options, callback);
          } else {
            return callback('Could not determine current Atom version installed');
          }
        };
      })(this));
    };

    Upgrade.prototype.upgradePackages = function(options, callback) {
      var packages;
      packages = this.getInstalledPackages(options);
      return this.getAvailableUpdates(packages, (function(_this) {
        return function(error, updates) {
          var packagesWithLatestVersion;
          if (error != null) {
            return callback(error);
          }
          if (options.argv.json) {
            packagesWithLatestVersion = updates.map(function(_arg) {
              var latestVersion, pack;
              pack = _arg.pack, latestVersion = _arg.latestVersion;
              pack.latestVersion = latestVersion;
              return pack;
            });
            console.log(JSON.stringify(packagesWithLatestVersion));
          } else {
            console.log("Package Updates Available".cyan + (" (" + updates.length + ")"));
            tree(updates, function(_arg) {
              var latestVersion, pack;
              pack = _arg.pack, latestVersion = _arg.latestVersion;
              return "" + pack.name.yellow + " " + pack.version.red + " -> " + latestVersion.green;
            });
          }
          if (options.command === 'outdated') {
            return callback();
          }
          if (options.argv.list) {
            return callback();
          }
          if (updates.length === 0) {
            return callback();
          }
          console.log();
          if (options.argv.confirm) {
            return _this.promptForConfirmation(function(error, confirmed) {
              if (error != null) {
                return callback(error);
              }
              if (confirmed) {
                console.log();
                return _this.installUpdates(updates, callback);
              } else {
                return callback();
              }
            });
          } else {
            return _this.installUpdates(updates, callback);
          }
        };
      })(this));
    };

    return Upgrade;

  })(Command);

}).call(this);
