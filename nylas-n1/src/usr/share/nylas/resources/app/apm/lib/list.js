(function() {
  var CSON, Command, List, config, fs, optimist, path, tree, _,
    __hasProp = {}.hasOwnProperty,
    __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

  path = require('path');

  _ = require('underscore-plus');

  CSON = require('season');

  optimist = require('optimist');

  Command = require('./command');

  fs = require('./fs');

  config = require('./apm');

  tree = require('./tree');

  module.exports = List = (function(_super) {
    __extends(List, _super);

    List.commandNames = ['list', 'ls'];

    function List() {
      var configPath, _ref, _ref1;
      this.userPackagesDirectory = path.join(config.getAtomDirectory(), 'packages');
      this.devPackagesDirectory = path.join(config.getAtomDirectory(), 'dev', 'packages');
      if (configPath = CSON.resolve(path.join(config.getAtomDirectory(), 'config'))) {
        try {
          this.disabledPackages = (_ref = CSON.readFileSync(configPath)) != null ? (_ref1 = _ref.core) != null ? _ref1.disabledPackages : void 0 : void 0;
        } catch (_error) {}
      }
      if (this.disabledPackages == null) {
        this.disabledPackages = [];
      }
    }

    List.prototype.parseOptions = function(argv) {
      var options;
      options = optimist(argv);
      options.usage("\nUsage: apm list\n       apm list --themes\n       apm list --packages\n       apm list --installed\n       apm list --installed --bare > my-packages.txt\n       apm list --json\n\nList all the installed packages and also the packages bundled with Atom.");
      options.alias('b', 'bare').boolean('bare').describe('bare', 'Print packages one per line with no formatting');
      options.alias('d', 'dev').boolean('dev')["default"]('dev', true).describe('dev', 'Include dev packages');
      options.alias('h', 'help').describe('help', 'Print this usage message');
      options.alias('i', 'installed').boolean('installed').describe('installed', 'Only list installed packages/themes');
      options.alias('j', 'json').boolean('json').describe('json', 'Output all packages as a JSON object');
      options.alias('l', 'links').boolean('links')["default"]('links', true).describe('links', 'Include linked packages');
      options.alias('t', 'themes').boolean('themes').describe('themes', 'Only list themes');
      return options.alias('p', 'packages').boolean('packages').describe('packages', 'Only list packages');
    };

    List.prototype.isPackageDisabled = function(name) {
      return this.disabledPackages.indexOf(name) !== -1;
    };

    List.prototype.logPackages = function(packages, options) {
      var pack, packageLine, _i, _len;
      if (options.argv.bare) {
        for (_i = 0, _len = packages.length; _i < _len; _i++) {
          pack = packages[_i];
          packageLine = pack.name;
          if (pack.version != null) {
            packageLine += "@" + pack.version;
          }
          console.log(packageLine);
        }
      } else {
        tree(packages, (function(_this) {
          return function(pack) {
            packageLine = pack.name;
            if (pack.version != null) {
              packageLine += "@" + pack.version;
            }
            if (_this.isPackageDisabled(pack.name)) {
              packageLine += ' (disabled)';
            }
            return packageLine;
          };
        })(this));
      }
      return console.log();
    };

    List.prototype.listPackages = function(directoryPath, options) {
      var child, manifest, manifestPath, packages, _i, _len, _ref;
      packages = [];
      _ref = fs.list(directoryPath);
      for (_i = 0, _len = _ref.length; _i < _len; _i++) {
        child = _ref[_i];
        if (!fs.isDirectorySync(path.join(directoryPath, child))) {
          continue;
        }
        if (!options.argv.links) {
          if (fs.isSymbolicLinkSync(path.join(directoryPath, child))) {
            continue;
          }
        }
        manifest = null;
        if (manifestPath = CSON.resolve(path.join(directoryPath, child, 'package'))) {
          try {
            manifest = CSON.readFileSync(manifestPath);
          } catch (_error) {}
        }
        if (manifest == null) {
          manifest = {};
        }
        manifest.name = child;
        if (options.argv.themes) {
          if (manifest.theme) {
            packages.push(manifest);
          }
        } else if (options.argv.packages) {
          if (!manifest.theme) {
            packages.push(manifest);
          }
        } else {
          packages.push(manifest);
        }
      }
      return packages;
    };

    List.prototype.listUserPackages = function(options, callback) {
      var userPackages;
      userPackages = this.listPackages(this.userPackagesDirectory, options);
      if (!(options.argv.bare || options.argv.json)) {
        console.log("" + this.userPackagesDirectory.cyan + " (" + userPackages.length + ")");
      }
      return typeof callback === "function" ? callback(null, userPackages) : void 0;
    };

    List.prototype.listDevPackages = function(options, callback) {
      var devPackages;
      if (!options.argv.dev) {
        return typeof callback === "function" ? callback(null, []) : void 0;
      }
      devPackages = this.listPackages(this.devPackagesDirectory, options);
      if (devPackages.length > 0) {
        if (!(options.argv.bare || options.argv.json)) {
          console.log("" + this.devPackagesDirectory.cyan + " (" + devPackages.length + ")");
        }
      }
      return typeof callback === "function" ? callback(null, devPackages) : void 0;
    };

    List.prototype.listBundledPackages = function(options, callback) {
      return config.getResourcePath((function(_this) {
        return function(resourcePath) {
          var metadata, metadataPath, packageName, packages, _atomPackages;
          try {
            metadataPath = path.join(resourcePath, 'package.json');
            _atomPackages = JSON.parse(fs.readFileSync(metadataPath))._atomPackages;
          } catch (_error) {}
          if (_atomPackages == null) {
            _atomPackages = {};
          }
          packages = (function() {
            var _results;
            _results = [];
            for (packageName in _atomPackages) {
              metadata = _atomPackages[packageName].metadata;
              _results.push(metadata);
            }
            return _results;
          })();
          packages = packages.filter(function(metadata) {
            if (options.argv.themes) {
              return metadata.theme;
            } else if (options.argv.packages) {
              return !metadata.theme;
            } else {
              return true;
            }
          });
          if (!(options.argv.bare || options.argv.json)) {
            if (options.argv.themes) {
              console.log("" + 'Built-in Atom themes'.cyan + " (" + packages.length + ")");
            } else {
              console.log("" + 'Built-in Atom packages'.cyan + " (" + packages.length + ")");
            }
          }
          return typeof callback === "function" ? callback(null, packages) : void 0;
        };
      })(this));
    };

    List.prototype.listInstalledPackages = function(options) {
      return this.listDevPackages(options, (function(_this) {
        return function(error, packages) {
          if (packages.length > 0) {
            _this.logPackages(packages, options);
          }
          return _this.listUserPackages(options, function(error, packages) {
            return _this.logPackages(packages, options);
          });
        };
      })(this));
    };

    List.prototype.listPackagesAsJson = function(options) {
      var output;
      output = {
        core: [],
        dev: [],
        user: []
      };
      return this.listBundledPackages(options, (function(_this) {
        return function(error, packages) {
          output.core = packages;
          return _this.listDevPackages(options, function(error, packages) {
            output.dev = packages;
            return _this.listUserPackages(options, function(error, packages) {
              output.user = packages;
              return console.log(JSON.stringify(output));
            });
          });
        };
      })(this));
    };

    List.prototype.run = function(options) {
      var callback;
      callback = options.callback;
      options = this.parseOptions(options.commandArgs);
      if (options.argv.json) {
        return this.listPackagesAsJson(options);
      } else if (options.argv.installed) {
        this.listInstalledPackages(options);
        return callback();
      } else {
        return this.listBundledPackages(options, (function(_this) {
          return function(error, packages) {
            _this.logPackages(packages, options);
            _this.listInstalledPackages(options);
            return callback();
          };
        })(this));
      }
    };

    return List;

  })(Command);

}).call(this);
