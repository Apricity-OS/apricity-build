(function() {
  var CSON, Command, Uninstall, async, auth, config, fs, optimist, path, request,
    __hasProp = {}.hasOwnProperty,
    __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

  path = require('path');

  async = require('async');

  CSON = require('season');

  optimist = require('optimist');

  auth = require('./auth');

  Command = require('./command');

  config = require('./apm');

  fs = require('./fs');

  request = require('./request');

  module.exports = Uninstall = (function(_super) {
    __extends(Uninstall, _super);

    function Uninstall() {
      return Uninstall.__super__.constructor.apply(this, arguments);
    }

    Uninstall.commandNames = ['deinstall', 'delete', 'erase', 'remove', 'rm', 'uninstall'];

    Uninstall.prototype.parseOptions = function(argv) {
      var options;
      options = optimist(argv);
      options.usage("\nUsage: apm uninstall <package_name>...\n\nDelete the installed package(s) from the ~/.atom/packages directory.");
      options.alias('h', 'help').describe('help', 'Print this usage message');
      options.alias('d', 'dev').boolean('dev').describe('dev', 'Uninstall from ~/.atom/dev/packages');
      return options.boolean('hard').describe('hard', 'Uninstall from ~/.atom/packages and ~/.atom/dev/packages');
    };

    Uninstall.prototype.getPackageVersion = function(packageDirectory) {
      var error, _ref;
      try {
        return (_ref = CSON.readFileSync(path.join(packageDirectory, 'package.json'))) != null ? _ref.version : void 0;
      } catch (_error) {
        error = _error;
        return null;
      }
    };

    Uninstall.prototype.registerUninstall = function(_arg, callback) {
      var packageName, packageVersion;
      packageName = _arg.packageName, packageVersion = _arg.packageVersion;
      if (!packageVersion) {
        return callback();
      }
      return auth.getToken(function(error, token) {
        var requestOptions;
        if (!token) {
          return callback();
        }
        requestOptions = {
          url: "" + (config.getAtomPackagesUrl()) + "/" + packageName + "/versions/" + packageVersion + "/events/uninstall",
          json: true,
          headers: {
            authorization: token
          }
        };
        return request.post(requestOptions, function(error, response, body) {
          return callback();
        });
      });
    };

    Uninstall.prototype.run = function(options) {
      var callback, devPackagesDirectory, error, packageDirectory, packageName, packageNames, packageVersion, packagesDirectory, uninstallError, uninstallsToRegister, _i, _len;
      callback = options.callback;
      options = this.parseOptions(options.commandArgs);
      packageNames = this.packageNamesFromArgv(options.argv);
      if (packageNames.length === 0) {
        callback("Please specify a package name to uninstall");
        return;
      }
      packagesDirectory = path.join(config.getAtomDirectory(), 'packages');
      devPackagesDirectory = path.join(config.getAtomDirectory(), 'dev', 'packages');
      uninstallsToRegister = [];
      uninstallError = null;
      for (_i = 0, _len = packageNames.length; _i < _len; _i++) {
        packageName = packageNames[_i];
        process.stdout.write("Uninstalling " + packageName + " ");
        try {
          if (!options.argv.dev) {
            packageDirectory = path.join(packagesDirectory, packageName);
            packageVersion = this.getPackageVersion(packageDirectory);
            fs.removeSync(packageDirectory);
            if (packageVersion) {
              uninstallsToRegister.push({
                packageName: packageName,
                packageVersion: packageVersion
              });
            }
          }
          if (options.argv.hard || options.argv.dev) {
            fs.removeSync(path.join(devPackagesDirectory, packageName));
          }
          this.logSuccess();
        } catch (_error) {
          error = _error;
          this.logFailure();
          uninstallError = new Error("Failed to delete " + packageName + ": " + error.message);
          break;
        }
      }
      return async.eachSeries(uninstallsToRegister, this.registerUninstall.bind(this), function() {
        return callback(uninstallError);
      });
    };

    return Uninstall;

  })(Command);

}).call(this);
