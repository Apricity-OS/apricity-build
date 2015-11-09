(function() {
  var Command, Config, apm, optimist, path, _,
    __hasProp = {}.hasOwnProperty,
    __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

  path = require('path');

  _ = require('underscore-plus');

  optimist = require('optimist');

  apm = require('./apm');

  Command = require('./command');

  module.exports = Config = (function(_super) {
    __extends(Config, _super);

    Config.commandNames = ['config'];

    function Config() {
      var atomDirectory;
      atomDirectory = apm.getAtomDirectory();
      this.atomNodeDirectory = path.join(atomDirectory, '.node-gyp');
      this.atomNpmPath = require.resolve('npm/bin/npm-cli');
    }

    Config.prototype.parseOptions = function(argv) {
      var options;
      options = optimist(argv);
      options.usage("\nUsage: apm config set <key> <value>\n       apm config get <key>\n       apm config delete <key>\n       apm config list\n       apm config edit\n");
      return options.alias('h', 'help').describe('help', 'Print this usage message');
    };

    Config.prototype.run = function(options) {
      var callback, configArgs, configOptions, env;
      callback = options.callback;
      options = this.parseOptions(options.commandArgs);
      configArgs = ['--globalconfig', apm.getGlobalConfigPath(), '--userconfig', apm.getUserConfigPath(), 'config'];
      configArgs = configArgs.concat(options.argv._);
      env = _.extend({}, process.env, {
        HOME: this.atomNodeDirectory
      });
      configOptions = {
        env: env
      };
      return this.fork(this.atomNpmPath, configArgs, configOptions, function(code, stderr, stdout) {
        if (stderr == null) {
          stderr = '';
        }
        if (stdout == null) {
          stdout = '';
        }
        if (code === 0) {
          if (stdout) {
            process.stdout.write(stdout);
          }
          return callback();
        } else {
          if (stderr) {
            process.stdout.write(stderr);
          }
          return callback(new Error("npm config failed: " + code));
        }
      });
    };

    return Config;

  })(Command);

}).call(this);
