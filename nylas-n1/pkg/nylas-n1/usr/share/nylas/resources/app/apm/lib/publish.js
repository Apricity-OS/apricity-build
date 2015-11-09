(function() {
  var Command, Git, Login, Packages, Publish, config, fs, optimist, path, request, semver, url,
    __hasProp = {}.hasOwnProperty,
    __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; },
    __slice = [].slice;

  path = require('path');

  url = require('url');

  optimist = require('optimist');

  Git = require('git-utils');

  semver = require('npm/node_modules/semver');

  fs = require('./fs');

  config = require('./apm');

  Command = require('./command');

  Login = require('./login');

  Packages = require('./packages');

  request = require('./request');

  module.exports = Publish = (function(_super) {
    __extends(Publish, _super);

    Publish.commandNames = ['publish'];

    function Publish() {
      this.userConfigPath = config.getUserConfigPath();
      this.atomNpmPath = require.resolve('npm/bin/npm-cli');
    }

    Publish.prototype.parseOptions = function(argv) {
      var options;
      options = optimist(argv);
      options.usage("\nUsage: apm publish [<newversion> | major | minor | patch | build]\n       apm publish --tag <tagname>\n       apm publish --rename <new-name>\n\nPublish a new version of the package in the current working directory.\n\nIf a new version or version increment is specified, then a new Git tag is\ncreated and the package.json file is updated with that new version before\nit is published to the apm registry. The HEAD branch and the new tag are\npushed up to the remote repository automatically using this option.\n\nIf a new name is provided via the --rename flag, the package.json file is\nupdated with the new name and the package's name is updated on Atom.io.\n\nRun `apm featured` to see all the featured packages or\n`apm view <packagename>` to see information about your package after you\nhave published it.");
      options.alias('h', 'help').describe('help', 'Print this usage message');
      options.alias('t', 'tag').string('tag').describe('tag', 'Specify a tag to publish');
      return options.alias('r', 'rename').string('rename').describe('rename', 'Specify a new name for the package');
    };

    Publish.prototype.versionPackage = function(version, callback) {
      var versionArgs;
      process.stdout.write('Preparing and tagging a new version ');
      versionArgs = ['version', version, '-m', 'Prepare %s release'];
      return this.fork(this.atomNpmPath, versionArgs, (function(_this) {
        return function(code, stderr, stdout) {
          if (stderr == null) {
            stderr = '';
          }
          if (stdout == null) {
            stdout = '';
          }
          if (code === 0) {
            _this.logSuccess();
            return callback(null, stdout.trim());
          } else {
            _this.logFailure();
            return callback(("" + stdout + "\n" + stderr).trim());
          }
        };
      })(this));
    };

    Publish.prototype.pushVersion = function(tag, callback) {
      var pushArgs;
      process.stdout.write("Pushing " + tag + " tag ");
      pushArgs = ['push', 'origin', 'HEAD', tag];
      return this.spawn('git', pushArgs, (function(_this) {
        return function() {
          var args;
          args = 1 <= arguments.length ? __slice.call(arguments, 0) : [];
          return _this.logCommandResults.apply(_this, [callback].concat(__slice.call(args)));
        };
      })(this));
    };

    Publish.prototype.waitForTagToBeAvailable = function(pack, tag, callback) {
      var interval, requestSettings, requestTags, retryCount;
      retryCount = 5;
      interval = 1000;
      requestSettings = {
        url: "https://api.github.com/repos/" + (Packages.getRepository(pack)) + "/tags",
        json: true
      };
      requestTags = function() {
        return request.get(requestSettings, function(error, response, tags) {
          var index, name, _i, _len;
          if (tags == null) {
            tags = [];
          }
          if ((response != null ? response.statusCode : void 0) === 200) {
            for (index = _i = 0, _len = tags.length; _i < _len; index = ++_i) {
              name = tags[index].name;
              if (name === tag) {
                return callback();
              }
            }
          }
          if (--retryCount <= 0) {
            return callback();
          } else {
            return setTimeout(requestTags, interval);
          }
        });
      };
      return requestTags();
    };

    Publish.prototype.packageExists = function(packageName, callback) {
      return Login.getTokenOrLogin(function(error, token) {
        var requestSettings;
        if (error != null) {
          return callback(error);
        }
        requestSettings = {
          url: "" + (config.getAtomPackagesUrl()) + "/" + packageName,
          json: true,
          headers: {
            authorization: token
          }
        };
        return request.get(requestSettings, function(error, response, body) {
          if (body == null) {
            body = {};
          }
          if (error != null) {
            return callback(error);
          } else {
            return callback(null, response.statusCode === 200);
          }
        });
      });
    };

    Publish.prototype.registerPackage = function(pack, callback) {
      if (!pack.name) {
        callback('Required name field in package.json not found');
        return;
      }
      return this.packageExists(pack.name, (function(_this) {
        return function(error, exists) {
          var repository;
          if (error != null) {
            return callback(error);
          }
          if (exists) {
            return callback();
          }
          if (!(repository = Packages.getRepository(pack))) {
            callback('Unable to parse repository name/owner from package.json repository field');
            return;
          }
          process.stdout.write("Registering " + pack.name + " ");
          return Login.getTokenOrLogin(function(error, token) {
            var requestSettings;
            if (error != null) {
              _this.logFailure();
              callback(error);
              return;
            }
            requestSettings = {
              url: config.getAtomPackagesUrl(),
              json: true,
              body: {
                repository: repository
              },
              headers: {
                authorization: token
              }
            };
            return request.post(requestSettings, function(error, response, body) {
              var message;
              if (body == null) {
                body = {};
              }
              if (error != null) {
                return callback(error);
              } else if (response.statusCode !== 201) {
                message = request.getErrorMessage(response, body);
                _this.logFailure();
                return callback("Registering package in " + repository + " repository failed: " + message);
              } else {
                _this.logSuccess();
                return callback(null, true);
              }
            });
          });
        };
      })(this));
    };

    Publish.prototype.createPackageVersion = function(packageName, tag, options, callback) {
      return Login.getTokenOrLogin(function(error, token) {
        var requestSettings;
        if (error != null) {
          callback(error);
          return;
        }
        requestSettings = {
          url: "" + (config.getAtomPackagesUrl()) + "/" + packageName + "/versions",
          json: true,
          body: {
            tag: tag,
            rename: options.rename
          },
          headers: {
            authorization: token
          }
        };
        return request.post(requestSettings, function(error, response, body) {
          var message;
          if (body == null) {
            body = {};
          }
          if (error != null) {
            return callback(error);
          } else if (response.statusCode !== 201) {
            message = request.getErrorMessage(response, body);
            return callback("Creating new version failed: " + message);
          } else {
            return callback();
          }
        });
      });
    };

    Publish.prototype.publishPackage = function() {
      var callback, options, pack, remaining, tag;
      pack = arguments[0], tag = arguments[1], remaining = 3 <= arguments.length ? __slice.call(arguments, 2) : [];
      if (remaining.length >= 2) {
        options = remaining.shift();
      }
      if (options == null) {
        options = {};
      }
      callback = remaining.shift();
      process.stdout.write("Publishing " + (options.rename || pack.name) + "@" + tag + " ");
      return this.createPackageVersion(pack.name, tag, options, (function(_this) {
        return function(error) {
          if (error != null) {
            _this.logFailure();
            return callback(error);
          } else {
            _this.logSuccess();
            return callback();
          }
        };
      })(this));
    };

    Publish.prototype.logFirstTimePublishMessage = function(pack) {
      process.stdout.write('Congrats on publishing a new package!'.rainbow);
      if (process.platform === 'darwin') {
        process.stdout.write(' \uD83D\uDC4D  \uD83D\uDCE6  \uD83C\uDF89');
      }
      return process.stdout.write("\nCheck it out at https://atom.io/packages/" + pack.name + "\n");
    };

    Publish.prototype.loadMetadata = function() {
      var error, metadataPath, pack;
      metadataPath = path.resolve('package.json');
      if (!fs.isFileSync(metadataPath)) {
        throw new Error("No package.json file found at " + (process.cwd()) + "/package.json");
      }
      try {
        return pack = JSON.parse(fs.readFileSync(metadataPath));
      } catch (_error) {
        error = _error;
        throw new Error("Error parsing package.json file: " + error.message);
      }
    };

    Publish.prototype.saveMetadata = function(pack, callback) {
      var metadataJson, metadataPath;
      metadataPath = path.resolve('package.json');
      metadataJson = JSON.stringify(pack, null, 2);
      return fs.writeFile(metadataPath, "" + metadataJson + "\n", callback);
    };

    Publish.prototype.loadRepository = function() {
      var currentBranch, currentDirectory, remoteName, repo, upstreamUrl;
      currentDirectory = process.cwd();
      repo = Git.open(currentDirectory);
      if (!(repo != null ? repo.isWorkingDirectory(currentDirectory) : void 0)) {
        throw new Error('Package must be in a Git repository before publishing: https://help.github.com/articles/create-a-repo');
      }
      if (currentBranch = repo.getShortHead()) {
        remoteName = repo.getConfigValue("branch." + currentBranch + ".remote");
      }
      if (remoteName == null) {
        remoteName = repo.getConfigValue('branch.master.remote');
      }
      if (remoteName) {
        upstreamUrl = repo.getConfigValue("remote." + remoteName + ".url");
      }
      if (upstreamUrl == null) {
        upstreamUrl = repo.getConfigValue('remote.origin.url');
      }
      if (!upstreamUrl) {
        throw new Error('Package must pushed up to GitHub before publishing: https://help.github.com/articles/create-a-repo');
      }
    };

    Publish.prototype.renamePackage = function(pack, name, callback) {
      var message;
      if ((name != null ? name.length : void 0) > 0) {
        if (pack.name === name) {
          return callback('The new package name must be different than the name in the package.json file');
        }
        message = "Renaming " + pack.name + " to " + name + " ";
        process.stdout.write(message);
        return this.setPackageName(pack, name, (function(_this) {
          return function(error) {
            if (error != null) {
              _this.logFailure();
              return callback(error);
            }
            return config.getSetting('git', function(gitCommand) {
              if (gitCommand == null) {
                gitCommand = 'git';
              }
              return _this.spawn(gitCommand, ['add', 'package.json'], function(code, stderr, stdout) {
                var addOutput;
                if (stderr == null) {
                  stderr = '';
                }
                if (stdout == null) {
                  stdout = '';
                }
                if (code !== 0) {
                  _this.logFailure();
                  addOutput = ("" + stdout + "\n" + stderr).trim();
                  return callback("`git add package.json` failed: " + addOutput);
                }
                return _this.spawn(gitCommand, ['commit', '-m', message], function(code, stderr, stdout) {
                  var commitOutput;
                  if (stderr == null) {
                    stderr = '';
                  }
                  if (stdout == null) {
                    stdout = '';
                  }
                  if (code === 0) {
                    _this.logSuccess();
                    return callback();
                  } else {
                    _this.logFailure();
                    commitOutput = ("" + stdout + "\n" + stderr).trim();
                    return callback("Failed to commit package.json: " + commitOutput);
                  }
                });
              });
            });
          };
        })(this));
      } else {
        return callback();
      }
    };

    Publish.prototype.setPackageName = function(pack, name, callback) {
      pack.name = name;
      return this.saveMetadata(pack, callback);
    };

    Publish.prototype.validateSemverRanges = function(pack) {
      var isValidRange, packageName, semverRange, _ref, _ref1, _ref2;
      if (!pack) {
        return;
      }
      isValidRange = function(semverRange) {
        if (semver.validRange(semverRange)) {
          return true;
        }
        try {
          if (url.parse(semverRange).protocol.length > 0) {
            return true;
          }
        } catch (_error) {}
        return semverRange === 'latest';
      };
      if (((_ref = pack.engines) != null ? _ref.atom : void 0) != null) {
        if (!semver.validRange(pack.engines.atom)) {
          throw new Error("The Atom engine range in the package.json file is invalid: " + pack.engines.atom);
        }
      }
      _ref1 = pack.dependencies;
      for (packageName in _ref1) {
        semverRange = _ref1[packageName];
        if (!isValidRange(semverRange)) {
          throw new Error("The " + packageName + " dependency range in the package.json file is invalid: " + semverRange);
        }
      }
      _ref2 = pack.devDependencies;
      for (packageName in _ref2) {
        semverRange = _ref2[packageName];
        if (!isValidRange(semverRange)) {
          throw new Error("The " + packageName + " dev dependency range in the package.json file is invalid: " + semverRange);
        }
      }
    };

    Publish.prototype.run = function(options) {
      var callback, error, originalName, pack, rename, tag, version, _ref;
      callback = options.callback;
      options = this.parseOptions(options.commandArgs);
      _ref = options.argv, tag = _ref.tag, rename = _ref.rename;
      version = options.argv._[0];
      try {
        pack = this.loadMetadata();
      } catch (_error) {
        error = _error;
        return callback(error);
      }
      try {
        this.validateSemverRanges(pack);
      } catch (_error) {
        error = _error;
        return callback(error);
      }
      try {
        this.loadRepository();
      } catch (_error) {
        error = _error;
        return callback(error);
      }
      if ((version != null ? version.length : void 0) > 0 || (rename != null ? rename.length : void 0) > 0) {
        if (!((version != null ? version.length : void 0) > 0)) {
          version = 'patch';
        }
        if ((rename != null ? rename.length : void 0) > 0) {
          originalName = pack.name;
        }
        return this.registerPackage(pack, (function(_this) {
          return function(error, firstTimePublishing) {
            if (error != null) {
              return callback(error);
            }
            return _this.renamePackage(pack, rename, function(error) {
              if (error != null) {
                return callback(error);
              }
              return _this.versionPackage(version, function(error, tag) {
                if (error != null) {
                  return callback(error);
                }
                return _this.pushVersion(tag, function(error) {
                  if (error != null) {
                    return callback(error);
                  }
                  return _this.waitForTagToBeAvailable(pack, tag, function() {
                    if (originalName != null) {
                      rename = pack.name;
                      pack.name = originalName;
                    }
                    return _this.publishPackage(pack, tag, {
                      rename: rename
                    }, function(error) {
                      if (firstTimePublishing && (error == null)) {
                        _this.logFirstTimePublishMessage(pack);
                      }
                      return callback(error);
                    });
                  });
                });
              });
            });
          };
        })(this));
      } else if ((tag != null ? tag.length : void 0) > 0) {
        return this.registerPackage(pack, (function(_this) {
          return function(error, firstTimePublishing) {
            if (error != null) {
              return callback(error);
            }
            return _this.publishPackage(pack, tag, function(error) {
              if (firstTimePublishing && (error == null)) {
                _this.logFirstTimePublishMessage(pack);
              }
              return callback(error);
            });
          };
        })(this));
      } else {
        return callback('A version, tag, or new package name is required');
      }
    };

    return Publish;

  })(Command);

}).call(this);
