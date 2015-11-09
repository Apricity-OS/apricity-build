(function() {
  var Command, Unpublish, auth, config, fs, optimist, path, readline, request,
    __hasProp = {}.hasOwnProperty,
    __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

  path = require('path');

  readline = require('readline');

  optimist = require('optimist');

  auth = require('./auth');

  Command = require('./command');

  config = require('./apm');

  fs = require('./fs');

  request = require('./request');

  module.exports = Unpublish = (function(_super) {
    __extends(Unpublish, _super);

    function Unpublish() {
      return Unpublish.__super__.constructor.apply(this, arguments);
    }

    Unpublish.commandNames = ['unpublish'];

    Unpublish.prototype.parseOptions = function(argv) {
      var options;
      options = optimist(argv);
      options.usage("Usage: apm unpublish [<package_name>]\n       apm unpublish <package_name>@<package_version>\n\nRemove a published package or package version from the atom.io registry.\n\nThe package in the current working directory will be used if no package\nname is specified.");
      options.alias('h', 'help').describe('help', 'Print this usage message');
      return options.alias('f', 'force').boolean('force').describe('force', 'Do not prompt for confirmation');
    };

    Unpublish.prototype.unpublishPackage = function(packageName, packageVersion, callback) {
      var packageLabel;
      packageLabel = packageName;
      if (packageVersion) {
        packageLabel += "@" + packageVersion;
      }
      process.stdout.write("Unpublishing " + packageLabel + " ");
      return auth.getToken((function(_this) {
        return function(error, token) {
          var options;
          if (error != null) {
            _this.logFailure();
            callback(error);
            return;
          }
          options = {
            uri: "" + (config.getAtomPackagesUrl()) + "/" + packageName,
            headers: {
              authorization: token
            },
            json: true
          };
          if (packageVersion) {
            options.uri += "/versions/" + packageVersion;
          }
          return request.del(options, function(error, response, body) {
            var message, _ref, _ref1;
            if (body == null) {
              body = {};
            }
            if (error != null) {
              _this.logFailure();
              return callback(error);
            } else if (response.statusCode !== 204) {
              _this.logFailure();
              message = (_ref = (_ref1 = body.message) != null ? _ref1 : body.error) != null ? _ref : body;
              return callback("Unpublishing failed: " + message);
            } else {
              _this.logSuccess();
              return callback();
            }
          });
        };
      })(this));
    };

    Unpublish.prototype.promptForConfirmation = function(packageName, packageVersion, callback) {
      var packageLabel, prompt;
      prompt = readline.createInterface(process.stdin, process.stdout);
      packageLabel = packageName;
      if (packageVersion) {
        packageLabel += "@" + packageVersion;
      }
      return prompt.question("Are you sure you want to unpublish " + packageLabel + "? (yes) ", (function(_this) {
        return function(answer) {
          prompt.close();
          answer = answer ? answer.trim().toLowerCase() : 'yes';
          if (answer === 'y' || answer === 'yes') {
            return _this.unpublishPackage(packageName, packageVersion, callback);
          }
        };
      })(this));
    };

    Unpublish.prototype.run = function(options) {
      var atIndex, callback, name, version, _ref;
      callback = options.callback;
      options = this.parseOptions(options.commandArgs);
      name = options.argv._[0];
      if ((name != null ? name.length : void 0) > 0) {
        atIndex = name.indexOf('@');
        if (atIndex !== -1) {
          version = name.substring(atIndex + 1);
          name = name.substring(0, atIndex);
        }
      }
      if (!name) {
        try {
          name = (_ref = JSON.parse(fs.readFileSync('package.json'))) != null ? _ref.name : void 0;
        } catch (_error) {}
      }
      if (!name) {
        name = path.basename(process.cwd());
      }
      if (options.argv.force) {
        return this.unpublishPackage(name, version, callback);
      } else {
        return this.promptForConfirmation(name, version, callback);
      }
    };

    return Unpublish;

  })(Command);

}).call(this);
