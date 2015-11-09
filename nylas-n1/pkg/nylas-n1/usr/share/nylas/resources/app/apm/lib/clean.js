(function() {
  var CSON, Clean, Command, async, config, fs, optimist, path, _,
    __hasProp = {}.hasOwnProperty,
    __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; },
    __slice = [].slice;

  path = require('path');

  async = require('async');

  CSON = require('season');

  optimist = require('optimist');

  _ = require('underscore-plus');

  Command = require('./command');

  config = require('./apm');

  fs = require('./fs');

  module.exports = Clean = (function(_super) {
    __extends(Clean, _super);

    Clean.commandNames = ['clean'];

    function Clean() {
      this.atomNpmPath = require.resolve('npm/bin/npm-cli');
    }

    Clean.prototype.getDependencies = function(modulePath, allDependencies) {
      var dependencies, error, installedModule, modulesPath, packageDependencies, _i, _len, _ref, _ref1, _ref2, _results;
      try {
        _ref1 = (_ref = CSON.readFileSync(CSON.resolve(path.join(modulePath, 'package')))) != null ? _ref : {}, dependencies = _ref1.dependencies, packageDependencies = _ref1.packageDependencies;
      } catch (_error) {
        error = _error;
        return;
      }
      _.extend(allDependencies, dependencies);
      modulesPath = path.join(modulePath, 'node_modules');
      _ref2 = fs.list(modulesPath);
      _results = [];
      for (_i = 0, _len = _ref2.length; _i < _len; _i++) {
        installedModule = _ref2[_i];
        if (installedModule !== '.bin') {
          _results.push(this.getDependencies(path.join(modulesPath, installedModule), allDependencies));
        }
      }
      return _results;
    };

    Clean.prototype.getModulesToRemove = function() {
      var dependencies, devDependencies, installedModule, installedModules, modulesPath, modulesToRemove, packageDependencies, packagePath, _i, _j, _len, _len1, _ref, _ref1;
      packagePath = CSON.resolve('package');
      if (!packagePath) {
        return [];
      }
      _ref1 = (_ref = CSON.readFileSync(packagePath)) != null ? _ref : {}, devDependencies = _ref1.devDependencies, dependencies = _ref1.dependencies, packageDependencies = _ref1.packageDependencies;
      if (devDependencies == null) {
        devDependencies = {};
      }
      if (dependencies == null) {
        dependencies = {};
      }
      if (packageDependencies == null) {
        packageDependencies = {};
      }
      modulesToRemove = [];
      modulesPath = path.resolve('node_modules');
      installedModules = fs.list(modulesPath).filter(function(modulePath) {
        return modulePath !== '.bin' && modulePath !== 'atom-package-manager';
      });
      for (_i = 0, _len = installedModules.length; _i < _len; _i++) {
        installedModule = installedModules[_i];
        this.getDependencies(path.join(modulesPath, installedModule), dependencies);
      }
      for (_j = 0, _len1 = installedModules.length; _j < _len1; _j++) {
        installedModule = installedModules[_j];
        if (dependencies.hasOwnProperty(installedModule)) {
          continue;
        }
        if (devDependencies.hasOwnProperty(installedModule)) {
          continue;
        }
        if (packageDependencies.hasOwnProperty(installedModule)) {
          continue;
        }
        modulesToRemove.push(installedModule);
      }
      return modulesToRemove;
    };

    Clean.prototype.parseOptions = function(argv) {
      var options;
      options = optimist(argv);
      options.usage("Usage: apm clean\n\nDeletes all packages in the node_modules folder that are not referenced\nas a dependency in the package.json file.");
      return options.alias('h', 'help').describe('help', 'Print this usage message');
    };

    Clean.prototype.removeModule = function(module, callback) {
      process.stdout.write("Removing " + module + " ");
      return this.fork(this.atomNpmPath, ['uninstall', module], (function(_this) {
        return function() {
          var args;
          args = 1 <= arguments.length ? __slice.call(arguments, 0) : [];
          return _this.logCommandResults.apply(_this, [callback].concat(__slice.call(args)));
        };
      })(this));
    };

    Clean.prototype.run = function(options) {
      var doneCallback, uninstallCommands;
      uninstallCommands = [];
      this.getModulesToRemove().forEach((function(_this) {
        return function(module) {
          return uninstallCommands.push(function(callback) {
            return _this.removeModule(module, callback);
          });
        };
      })(this));
      if (uninstallCommands.length > 0) {
        doneCallback = (function(_this) {
          return function(error) {
            if (error != null) {
              return options.callback(error);
            } else {
              return _this.run(options);
            }
          };
        })(this);
      } else {
        doneCallback = options.callback;
      }
      return async.waterfall(uninstallCommands, doneCallback);
    };

    return Clean;

  })(Command);

}).call(this);
