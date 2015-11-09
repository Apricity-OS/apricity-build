(function() {
  var Command, Develop, Install, Link, config, fs, git, optimist, path, request, _,
    __hasProp = {}.hasOwnProperty,
    __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

  fs = require('fs');

  path = require('path');

  _ = require('underscore-plus');

  optimist = require('optimist');

  config = require('./apm');

  Command = require('./command');

  Install = require('./install');

  git = require('./git');

  Link = require('./link');

  request = require('./request');

  module.exports = Develop = (function(_super) {
    __extends(Develop, _super);

    Develop.commandNames = ['dev', 'develop'];

    function Develop() {
      this.atomDirectory = config.getAtomDirectory();
      this.atomDevPackagesDirectory = path.join(this.atomDirectory, 'dev', 'packages');
    }

    Develop.prototype.parseOptions = function(argv) {
      var options;
      options = optimist(argv);
      options.usage("Usage: apm develop <package_name> [<directory>]\n\nClone the given package's Git repository to the directory specified,\ninstall its dependencies, and link it for development to\n~/.atom/dev/packages/<package_name>.\n\nIf no directory is specified then the repository is cloned to\n~/github/<package_name>. The default folder to clone packages into can\nbe overridden using the ATOM_REPOS_HOME environment variable.\n\nOnce this command completes you can open a dev window from atom using\ncmd-shift-o to run the package out of the newly cloned repository.");
      return options.alias('h', 'help').describe('help', 'Print this usage message');
    };

    Develop.prototype.getRepositoryUrl = function(packageName, callback) {
      var requestSettings;
      requestSettings = {
        url: "" + (config.getAtomPackagesUrl()) + "/" + packageName,
        json: true
      };
      return request.get(requestSettings, function(error, response, body) {
        var message, repositoryUrl;
        if (body == null) {
          body = {};
        }
        if (error != null) {
          return callback("Request for package information failed: " + error.message);
        } else if (response.statusCode === 200) {
          if (repositoryUrl = body.repository.url) {
            return callback(null, repositoryUrl);
          } else {
            return callback("No repository URL found for package: " + packageName);
          }
        } else {
          message = request.getErrorMessage(response, body);
          return callback("Request for package information failed: " + message);
        }
      });
    };

    Develop.prototype.cloneRepository = function(repoUrl, packageDirectory, options) {
      return config.getSetting('git', (function(_this) {
        return function(command) {
          var args;
          if (command == null) {
            command = 'git';
          }
          args = ['clone', '--recursive', repoUrl, packageDirectory];
          process.stdout.write("Cloning " + repoUrl + " ");
          git.addGitToEnv(process.env);
          return _this.spawn(command, args, function(code, stderr, stdout) {
            if (stderr == null) {
              stderr = '';
            }
            if (stdout == null) {
              stdout = '';
            }
            if (code === 0) {
              _this.logSuccess();
              return _this.installDependencies(packageDirectory, options);
            } else {
              _this.logFailure();
              return options.callback(("" + stdout + "\n" + stderr).trim());
            }
          });
        };
      })(this));
    };

    Develop.prototype.installDependencies = function(packageDirectory, options) {
      var installOptions;
      process.chdir(packageDirectory);
      installOptions = _.clone(options);
      installOptions.callback = (function(_this) {
        return function(error) {
          if (error != null) {
            return options.callback(error);
          } else {
            return _this.linkPackage(packageDirectory, options);
          }
        };
      })(this);
      return new Install().run(installOptions);
    };

    Develop.prototype.linkPackage = function(packageDirectory, options) {
      var linkOptions;
      linkOptions = _.clone(options);
      linkOptions.commandArgs = [packageDirectory, '--dev'];
      return new Link().run(linkOptions);
    };

    Develop.prototype.run = function(options) {
      var packageDirectory, packageName, _ref;
      packageName = options.commandArgs.shift();
      if (!((packageName != null ? packageName.length : void 0) > 0)) {
        return options.callback("Missing required package name");
      }
      packageDirectory = (_ref = options.commandArgs.shift()) != null ? _ref : path.join(config.getReposDirectory(), packageName);
      packageDirectory = path.resolve(packageDirectory);
      if (fs.existsSync(packageDirectory)) {
        return this.linkPackage(packageDirectory, options);
      } else {
        return this.getRepositoryUrl(packageName, (function(_this) {
          return function(error, repoUrl) {
            if (error != null) {
              return options.callback(error);
            } else {
              return _this.cloneRepository(repoUrl, packageDirectory, options);
            }
          };
        })(this));
      }
    };

    return Develop;

  })(Command);

}).call(this);
