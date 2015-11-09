(function() {
  var Command, Install, Rebuild, config, optimist, path, _,
    __hasProp = {}.hasOwnProperty,
    __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

  path = require('path');

  _ = require('underscore-plus');

  optimist = require('optimist');

  config = require('./apm');

  Command = require('./command');

  Install = require('./install');

  module.exports = Rebuild = (function(_super) {
    __extends(Rebuild, _super);

    Rebuild.commandNames = ['rebuild'];

    function Rebuild() {
      this.atomNodeDirectory = path.join(config.getAtomDirectory(), '.node-gyp');
      this.atomNpmPath = require.resolve('npm/bin/npm-cli');
    }

    Rebuild.prototype.parseOptions = function(argv) {
      var options;
      options = optimist(argv);
      options.usage("\nUsage: apm rebuild [<name> [<name> ...]]\n\nRebuild the given modules currently installed in the node_modules folder\nin the current working directory.\n\nAll the modules will be rebuilt if no module names are specified.");
      return options.alias('h', 'help').describe('help', 'Print this usage message');
    };

    Rebuild.prototype.run = function(options) {
      var callback;
      callback = options.callback;
      options = this.parseOptions(options.commandArgs);
      return config.loadNpm((function(_this) {
        return function(error, npm) {
          var install;
          install = new Install();
          install.npm = npm;
          return install.installNode(function(error) {
            var env, rebuildArgs;
            if (error != null) {
              return callback(error);
            }
            process.stdout.write('Rebuilding modules ');
            rebuildArgs = ['--globalconfig', config.getGlobalConfigPath(), '--userconfig', config.getUserConfigPath(), 'rebuild'];
            rebuildArgs.push("--target=" + (config.getNodeVersion()));
            rebuildArgs.push("--arch=" + (config.getNodeArch()));
            rebuildArgs = rebuildArgs.concat(options.argv._);
            env = _.extend({}, process.env, {
              HOME: _this.atomNodeDirectory
            });
            if (config.isWin32()) {
              env.USERPROFILE = env.HOME;
            }
            return _this.fork(_this.atomNpmPath, rebuildArgs, {
              env: env
            }, function(code, stderr) {
              if (stderr == null) {
                stderr = '';
              }
              if (code === 0) {
                _this.logSuccess();
                return callback();
              } else {
                _this.logFailure();
                return callback(stderr);
              }
            });
          });
        };
      })(this));
    };

    return Rebuild;

  })(Command);

}).call(this);
