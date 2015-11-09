(function() {
  var Command, Dedupe, async, config, fs, optimist, path, _,
    __hasProp = {}.hasOwnProperty,
    __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; },
    __slice = [].slice;

  path = require('path');

  async = require('async');

  _ = require('underscore-plus');

  optimist = require('optimist');

  config = require('./apm');

  Command = require('./command');

  fs = require('./fs');

  module.exports = Dedupe = (function(_super) {
    __extends(Dedupe, _super);

    Dedupe.commandNames = ['dedupe'];

    function Dedupe() {
      this.atomDirectory = config.getAtomDirectory();
      this.atomPackagesDirectory = path.join(this.atomDirectory, 'packages');
      this.atomNodeDirectory = path.join(this.atomDirectory, '.node-gyp');
      this.atomNpmPath = require.resolve('npm/bin/npm-cli');
      this.atomNodeGypPath = require.resolve('npm/node_modules/node-gyp/bin/node-gyp');
    }

    Dedupe.prototype.parseOptions = function(argv) {
      var options;
      options = optimist(argv);
      options.usage("\nUsage: apm dedupe [<package_name>...]\n\nReduce duplication in the node_modules folder in the current directory.\n\nThis command is experimental.");
      return options.alias('h', 'help').describe('help', 'Print this usage message');
    };

    Dedupe.prototype.installNode = function(callback) {
      var env, installNodeArgs;
      installNodeArgs = ['install'];
      installNodeArgs.push("--target=" + (config.getNodeVersion()));
      installNodeArgs.push("--dist-url=" + (config.getNodeUrl()));
      installNodeArgs.push('--arch=ia32');
      installNodeArgs.push('--ensure');
      env = _.extend({}, process.env, {
        HOME: this.atomNodeDirectory
      });
      if (config.isWin32()) {
        env.USERPROFILE = env.HOME;
      }
      fs.makeTreeSync(this.atomDirectory);
      return config.loadNpm((function(_this) {
        return function(error, npm) {
          var proxy, useStrictSsl, _ref;
          useStrictSsl = (_ref = npm.config.get('strict-ssl')) != null ? _ref : true;
          if (!useStrictSsl) {
            env.NODE_TLS_REJECT_UNAUTHORIZED = 0;
          }
          proxy = npm.config.get('https-proxy') || npm.config.get('proxy');
          if (proxy) {
            installNodeArgs.push("--proxy=" + proxy);
          }
          return _this.fork(_this.atomNodeGypPath, installNodeArgs, {
            env: env,
            cwd: _this.atomDirectory
          }, function(code, stderr, stdout) {
            if (stderr == null) {
              stderr = '';
            }
            if (stdout == null) {
              stdout = '';
            }
            if (code === 0) {
              return callback();
            } else {
              return callback("" + stdout + "\n" + stderr);
            }
          });
        };
      })(this));
    };

    Dedupe.prototype.getVisualStudioFlags = function() {
      var vsVersion;
      if (vsVersion = config.getInstalledVisualStudioFlag()) {
        return "--msvs_version=" + vsVersion;
      }
    };

    Dedupe.prototype.dedupeModules = function(options, callback) {
      process.stdout.write('Deduping modules ');
      return this.forkDedupeCommand(options, (function(_this) {
        return function() {
          var args;
          args = 1 <= arguments.length ? __slice.call(arguments, 0) : [];
          return _this.logCommandResults.apply(_this, [callback].concat(__slice.call(args)));
        };
      })(this));
    };

    Dedupe.prototype.forkDedupeCommand = function(options, callback) {
      var dedupeArgs, dedupeOptions, env, packageName, vsArgs, _i, _len, _ref;
      dedupeArgs = ['--globalconfig', config.getGlobalConfigPath(), '--userconfig', config.getUserConfigPath(), 'dedupe'];
      dedupeArgs.push("--target=" + (config.getNodeVersion()));
      dedupeArgs.push('--arch=ia32');
      if (options.argv.silent) {
        dedupeArgs.push('--silent');
      }
      if (options.argv.quiet) {
        dedupeArgs.push('--quiet');
      }
      if (vsArgs = this.getVisualStudioFlags()) {
        dedupeArgs.push(vsArgs);
      }
      _ref = options.argv._;
      for (_i = 0, _len = _ref.length; _i < _len; _i++) {
        packageName = _ref[_i];
        dedupeArgs.push(packageName);
      }
      env = _.extend({}, process.env, {
        HOME: this.atomNodeDirectory
      });
      if (config.isWin32()) {
        env.USERPROFILE = env.HOME;
      }
      dedupeOptions = {
        env: env
      };
      if (options.cwd) {
        dedupeOptions.cwd = options.cwd;
      }
      return this.fork(this.atomNpmPath, dedupeArgs, dedupeOptions, callback);
    };

    Dedupe.prototype.createAtomDirectories = function() {
      fs.makeTreeSync(this.atomDirectory);
      return fs.makeTreeSync(this.atomNodeDirectory);
    };

    Dedupe.prototype.run = function(options) {
      var callback, commands, cwd;
      callback = options.callback, cwd = options.cwd;
      options = this.parseOptions(options.commandArgs);
      options.cwd = cwd;
      this.createAtomDirectories();
      commands = [];
      commands.push((function(_this) {
        return function(callback) {
          return _this.installNode(callback);
        };
      })(this));
      commands.push((function(_this) {
        return function(callback) {
          return _this.dedupeModules(options, callback);
        };
      })(this));
      return async.waterfall(commands, callback);
    };

    return Dedupe;

  })(Command);

}).call(this);
