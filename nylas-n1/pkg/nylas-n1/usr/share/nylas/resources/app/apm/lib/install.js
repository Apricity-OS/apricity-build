(function() {
  var CSON, Command, Install, RebuildModuleCache, async, config, fs, git, optimist, path, request, semver, temp, _,
    __bind = function(fn, me){ return function(){ return fn.apply(me, arguments); }; },
    __hasProp = {}.hasOwnProperty,
    __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; },
    __slice = [].slice;

  path = require('path');

  async = require('async');

  _ = require('underscore-plus');

  optimist = require('optimist');

  CSON = require('season');

  semver = require('npm/node_modules/semver');

  temp = require('temp');

  config = require('./apm');

  Command = require('./command');

  fs = require('./fs');

  git = require('./git');

  RebuildModuleCache = require('./rebuild-module-cache');

  request = require('./request');

  module.exports = Install = (function(_super) {
    __extends(Install, _super);

    Install.commandNames = ['install'];

    function Install() {
      this.installModules = __bind(this.installModules, this);
      this.installNode = __bind(this.installNode, this);
      this.atomDirectory = config.getAtomDirectory();
      this.atomPackagesDirectory = path.join(this.atomDirectory, 'packages');
      this.atomNodeDirectory = path.join(this.atomDirectory, '.node-gyp');
      this.atomNpmPath = require.resolve('npm/bin/npm-cli');
      this.atomNodeGypPath = require.resolve('npm/node_modules/node-gyp/bin/node-gyp');
    }

    Install.prototype.parseOptions = function(argv) {
      var options;
      options = optimist(argv);
      options.usage("\nUsage: apm install [<package_name>...]\n       apm install <package_name>@<package_version>\n       apm install --packages-file my-packages.txt\n\nInstall the given Atom package to ~/.atom/packages/<package_name>.\n\nIf no package name is given then all the dependencies in the package.json\nfile are installed to the node_modules folder in the current working\ndirectory.\n\nA packages file can be specified that is a newline separated list of\npackage names to install with optional versions using the\n`package-name@version` syntax.");
      options.alias('c', 'compatible').string('compatible').describe('compatible', 'Only install packages/themes compatible with this Atom version');
      options.alias('h', 'help').describe('help', 'Print this usage message');
      options.alias('s', 'silent').boolean('silent').describe('silent', 'Set the npm log level to silent');
      options.alias('q', 'quiet').boolean('quiet').describe('quiet', 'Set the npm log level to warn');
      options.boolean('check').describe('check', 'Check that native build tools are installed');
      options.boolean('verbose')["default"]('verbose', false).describe('verbose', 'Show verbose debug information');
      options.string('packages-file').describe('packages-file', 'A text file containing the packages to install');
      return options.boolean('production').describe('production', 'Do not install dev dependencies');
    };

    Install.prototype.installNode = function(callback) {
      var env, installNodeArgs, opts, proxy, useStrictSsl, _ref;
      installNodeArgs = ['install'];
      installNodeArgs.push("--target=" + (config.getNodeVersion()));
      installNodeArgs.push("--dist-url=" + (config.getNodeUrl()));
      installNodeArgs.push("--arch=" + (config.getNodeArch()));
      installNodeArgs.push("--ensure");
      if (this.verbose) {
        installNodeArgs.push("--verbose");
      }
      env = _.extend({}, process.env, {
        HOME: this.atomNodeDirectory
      });
      if (config.isWin32()) {
        env.USERPROFILE = env.HOME;
      }
      fs.makeTreeSync(this.atomDirectory);
      useStrictSsl = (_ref = this.npm.config.get('strict-ssl')) != null ? _ref : true;
      if (!useStrictSsl) {
        env.NODE_TLS_REJECT_UNAUTHORIZED = 0;
      }
      proxy = this.npm.config.get('https-proxy') || this.npm.config.get('proxy');
      if (proxy) {
        installNodeArgs.push("--proxy=" + proxy);
      }
      opts = {
        env: env,
        cwd: this.atomDirectory
      };
      if (this.verbose) {
        opts.streaming = true;
      }
      return this.fork(this.atomNodeGypPath, installNodeArgs, opts, function(code, stderr, stdout) {
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

    Install.prototype.updateWindowsEnv = function(env) {
      var localModuleBins;
      env.USERPROFILE = env.HOME;
      localModuleBins = path.resolve(__dirname, '..', 'node_modules', '.bin');
      if (env.Path) {
        env.Path += "" + path.delimiter + localModuleBins;
      } else {
        env.Path = localModuleBins;
      }
      return git.addGitToEnv(env);
    };

    Install.prototype.addNodeBinToEnv = function(env) {
      var nodeBinFolder, pathKey;
      nodeBinFolder = path.resolve(__dirname, '..', 'bin');
      pathKey = config.isWin32() ? 'Path' : 'PATH';
      if (env[pathKey]) {
        return env[pathKey] = "" + nodeBinFolder + path.delimiter + env[pathKey];
      } else {
        return env[pathKey] = nodeBinFolder;
      }
    };

    Install.prototype.addProxyToEnv = function(env) {
      var httpProxy, httpsProxy;
      httpProxy = this.npm.config.get('proxy');
      if (httpProxy) {
        if (env.HTTP_PROXY == null) {
          env.HTTP_PROXY = httpProxy;
        }
        if (env.http_proxy == null) {
          env.http_proxy = httpProxy;
        }
      }
      httpsProxy = this.npm.config.get('https-proxy');
      if (httpsProxy) {
        if (env.HTTPS_PROXY == null) {
          env.HTTPS_PROXY = httpsProxy;
        }
        return env.https_proxy != null ? env.https_proxy : env.https_proxy = httpsProxy;
      }
    };

    Install.prototype.installModule = function(options, pack, modulePath, callback) {
      var env, installArgs, installDirectory, installGlobally, installOptions, nodeModulesDirectory, vsArgs, _ref;
      installArgs = ['--globalconfig', config.getGlobalConfigPath(), '--userconfig', config.getUserConfigPath(), 'install'];
      installArgs.push(modulePath);
      installArgs.push("--target=" + (config.getNodeVersion()));
      installArgs.push("--arch=" + (config.getNodeArch()));
      if (options.argv.silent) {
        installArgs.push('--silent');
      }
      if (options.argv.quiet) {
        installArgs.push('--quiet');
      }
      if (options.argv.production) {
        installArgs.push('--production');
      }
      if (vsArgs = this.getVisualStudioFlags()) {
        installArgs.push(vsArgs);
      }
      env = _.extend({}, process.env, {
        HOME: this.atomNodeDirectory
      });
      if (config.isWin32()) {
        this.updateWindowsEnv(env);
      }
      this.addNodeBinToEnv(env);
      this.addProxyToEnv(env);
      installOptions = {
        env: env
      };
      if (this.verbose) {
        installOptions.streaming = true;
      }
      installGlobally = (_ref = options.installGlobally) != null ? _ref : true;
      if (installGlobally) {
        installDirectory = temp.mkdirSync('apm-install-dir-');
        nodeModulesDirectory = path.join(installDirectory, 'node_modules');
        fs.makeTreeSync(nodeModulesDirectory);
        installOptions.cwd = installDirectory;
      }
      return this.fork(this.atomNpmPath, installArgs, installOptions, (function(_this) {
        return function(code, stderr, stdout) {
          var child, commands, destination, error, source, _fn, _i, _len, _ref1;
          if (stderr == null) {
            stderr = '';
          }
          if (stdout == null) {
            stdout = '';
          }
          if (code === 0) {
            if (installGlobally) {
              commands = [];
              _ref1 = fs.readdirSync(nodeModulesDirectory);
              _fn = function(source, destination) {
                return commands.push(function(callback) {
                  return fs.cp(source, destination, callback);
                });
              };
              for (_i = 0, _len = _ref1.length; _i < _len; _i++) {
                child = _ref1[_i];
                source = path.join(nodeModulesDirectory, child);
                destination = path.join(_this.atomPackagesDirectory, child);
                _fn(source, destination);
              }
              commands.push(function(callback) {
                return _this.buildModuleCache(pack.name, callback);
              });
              commands.push(function(callback) {
                return _this.warmCompileCache(pack.name, callback);
              });
              return async.waterfall(commands, function(error) {
                if (error != null) {
                  _this.logFailure();
                } else {
                  _this.logSuccess();
                }
                return callback(error);
              });
            } else {
              return callback();
            }
          } else {
            if (installGlobally) {
              fs.removeSync(installDirectory);
              _this.logFailure();
            }
            error = "" + stdout + "\n" + stderr;
            if (error.indexOf('code ENOGIT') !== -1) {
              error = _this.getGitErrorMessage(pack);
            }
            return callback(error);
          }
        };
      })(this));
    };

    Install.prototype.getGitErrorMessage = function(pack) {
      var message;
      message = "Failed to install " + pack.name + " because Git was not found.\n\nThe " + pack.name + " package has module dependencies that cannot be installed without Git.\n\nYou need to install Git and add it to your path environment variable in order to install this package.\n";
      switch (process.platform) {
        case 'win32':
          message += "\nYou can install Git by downloading, installing, and launching GitHub for Windows: https://windows.github.com\n";
          break;
        case 'linux':
          message += "\nYou can install Git from your OS package manager.\n";
      }
      message += "\nRun apm -v after installing Git to see what version has been detected.";
      return message;
    };

    Install.prototype.getVisualStudioFlags = function() {
      var vsVersion;
      if (vsVersion = config.getInstalledVisualStudioFlag()) {
        return "--msvs_version=" + vsVersion;
      }
    };

    Install.prototype.installModules = function(options, callback) {
      process.stdout.write('Installing modules ');
      return this.forkInstallCommand(options, (function(_this) {
        return function() {
          var args;
          args = 1 <= arguments.length ? __slice.call(arguments, 0) : [];
          return _this.logCommandResults.apply(_this, [callback].concat(__slice.call(args)));
        };
      })(this));
    };

    Install.prototype.forkInstallCommand = function(options, callback) {
      var env, installArgs, installOptions, vsArgs;
      installArgs = ['--globalconfig', config.getGlobalConfigPath(), '--userconfig', config.getUserConfigPath(), 'install'];
      installArgs.push("--target=" + (config.getNodeVersion()));
      installArgs.push("--arch=" + (config.getNodeArch()));
      if (options.argv.silent) {
        installArgs.push('--silent');
      }
      if (options.argv.quiet) {
        installArgs.push('--quiet');
      }
      if (options.argv.production) {
        installArgs.push('--production');
      }
      if (vsArgs = this.getVisualStudioFlags()) {
        installArgs.push(vsArgs);
      }
      env = _.extend({}, process.env, {
        HOME: this.atomNodeDirectory
      });
      if (config.isWin32()) {
        this.updateWindowsEnv(env);
      }
      this.addNodeBinToEnv(env);
      this.addProxyToEnv(env);
      installOptions = {
        env: env
      };
      if (options.cwd) {
        installOptions.cwd = options.cwd;
      }
      if (this.verbose) {
        installOptions.streaming = true;
      }
      return this.fork(this.atomNpmPath, installArgs, installOptions, callback);
    };

    Install.prototype.requestPackage = function(packageName, callback) {
      var requestSettings;
      requestSettings = {
        url: "" + (config.getAtomPackagesUrl()) + "/" + packageName,
        json: true,
        retries: 4
      };
      return request.get(requestSettings, function(error, response, body) {
        var message;
        if (body == null) {
          body = {};
        }
        if (error != null) {
          message = "Request for package information failed: " + error.message;
          if (error.code) {
            message += " (" + error.code + ")";
          }
          return callback(message);
        } else if (response.statusCode !== 200) {
          message = request.getErrorMessage(response, body);
          return callback("Request for package information failed: " + message);
        } else {
          if (body.releases.latest) {
            return callback(null, body);
          } else {
            return callback("No releases available for " + packageName);
          }
        }
      });
    };

    Install.prototype.downloadPackage = function(packageUrl, installGlobally, callback) {
      var requestSettings;
      requestSettings = {
        url: packageUrl
      };
      return request.createReadStream(requestSettings, (function(_this) {
        return function(readStream) {
          readStream.on('error', function(error) {
            return callback("Unable to download " + packageUrl + ": " + error.message);
          });
          return readStream.on('response', function(response) {
            var chunks, filePath, writeStream;
            if (response.statusCode === 200) {
              filePath = path.join(temp.mkdirSync(), 'package.tgz');
              writeStream = fs.createWriteStream(filePath);
              readStream.pipe(writeStream);
              writeStream.on('error', function(error) {
                return callback("Unable to download " + packageUrl + ": " + error.message);
              });
              return writeStream.on('close', function() {
                return callback(null, filePath);
              });
            } else {
              chunks = [];
              response.on('data', function(chunk) {
                return chunks.push(chunk);
              });
              return response.on('end', function() {
                var error, message, parseError, _ref, _ref1, _ref2, _ref3;
                try {
                  error = JSON.parse(Buffer.concat(chunks));
                  message = (_ref = (_ref1 = error.message) != null ? _ref1 : error.error) != null ? _ref : error;
                  if (installGlobally) {
                    _this.logFailure();
                  }
                  return callback("Unable to download " + packageUrl + ": " + ((_ref2 = response.headers.status) != null ? _ref2 : response.statusCode) + " " + message);
                } catch (_error) {
                  parseError = _error;
                  if (installGlobally) {
                    _this.logFailure();
                  }
                  return callback("Unable to download " + packageUrl + ": " + ((_ref3 = response.headers.status) != null ? _ref3 : response.statusCode));
                }
              });
            }
          });
        };
      })(this));
    };

    Install.prototype.getPackageCachePath = function(packageName, packageVersion, callback) {
      var cacheDir, cachePath, tempPath;
      cacheDir = config.getCacheDirectory();
      cachePath = path.join(cacheDir, packageName, packageVersion, 'package.tgz');
      if (fs.isFileSync(cachePath)) {
        tempPath = path.join(temp.mkdirSync(), path.basename(cachePath));
        return fs.cp(cachePath, tempPath, function(error) {
          if (error != null) {
            return callback(error);
          } else {
            return callback(null, tempPath);
          }
        });
      } else {
        return process.nextTick(function() {
          return callback(new Error("" + packageName + "@" + packageVersion + " is not in the cache"));
        });
      }
    };

    Install.prototype.isPackageInstalled = function(packageName, packageVersion) {
      var error, version, _ref;
      try {
        version = ((_ref = CSON.readFileSync(CSON.resolve(path.join('node_modules', packageName, 'package')))) != null ? _ref : {}).version;
        return packageVersion === version;
      } catch (_error) {
        error = _error;
        return false;
      }
    };

    Install.prototype.installPackage = function(metadata, options, callback) {
      var installGlobally, label, packageName, packageVersion, _ref;
      packageName = metadata.name;
      packageVersion = metadata.version;
      installGlobally = (_ref = options.installGlobally) != null ? _ref : true;
      if (!installGlobally) {
        if (packageVersion && this.isPackageInstalled(packageName, packageVersion)) {
          callback();
          return;
        }
      }
      label = packageName;
      if (packageVersion) {
        label += "@" + packageVersion;
      }
      process.stdout.write("Installing " + label + " ");
      if (installGlobally) {
        process.stdout.write("to " + this.atomPackagesDirectory + " ");
      }
      return this.requestPackage(packageName, (function(_this) {
        return function(error, pack) {
          var commands, installNode, tarball, _ref1, _ref2, _ref3;
          if (error != null) {
            _this.logFailure();
            return callback(error);
          } else {
            if (packageVersion == null) {
              packageVersion = _this.getLatestCompatibleVersion(pack);
            }
            if (!packageVersion) {
              _this.logFailure();
              callback("No available version compatible with the installed Atom version: " + _this.installedAtomVersion);
            }
            tarball = ((_ref1 = (_ref2 = pack.versions[packageVersion]) != null ? _ref2.dist : void 0) != null ? _ref1 : {}).tarball;
            if (!tarball) {
              _this.logFailure();
              callback("Package version: " + packageVersion + " not found");
              return;
            }
            commands = [];
            commands.push(function(callback) {
              return _this.getPackageCachePath(packageName, packageVersion, function(error, packagePath) {
                if (packagePath) {
                  return callback(null, packagePath);
                } else {
                  return _this.downloadPackage(tarball, installGlobally, callback);
                }
              });
            });
            installNode = (_ref3 = options.installNode) != null ? _ref3 : true;
            if (installNode) {
              commands.push(function(packagePath, callback) {
                return _this.installNode(function(error) {
                  return callback(error, packagePath);
                });
              });
            }
            commands.push(function(packagePath, callback) {
              return _this.installModule(options, pack, packagePath, callback);
            });
            return async.waterfall(commands, function(error) {
              if (!installGlobally) {
                if (error != null) {
                  _this.logFailure();
                } else {
                  _this.logSuccess();
                }
              }
              return callback(error);
            });
          }
        };
      })(this));
    };

    Install.prototype.installPackageDependencies = function(options, callback) {
      var commands, name, version, _fn, _ref;
      options = _.extend({}, options, {
        installGlobally: false,
        installNode: false
      });
      commands = [];
      _ref = this.getPackageDependencies();
      _fn = (function(_this) {
        return function(name, version) {
          return commands.push(function(callback) {
            return _this.installPackage({
              name: name,
              version: version
            }, options, callback);
          });
        };
      })(this);
      for (name in _ref) {
        version = _ref[name];
        _fn(name, version);
      }
      return async.waterfall(commands, callback);
    };

    Install.prototype.installDependencies = function(options, callback) {
      var commands;
      options.installGlobally = false;
      commands = [];
      commands.push(this.installNode);
      commands.push((function(_this) {
        return function(callback) {
          return _this.installModules(options, callback);
        };
      })(this));
      commands.push((function(_this) {
        return function(callback) {
          return _this.installPackageDependencies(options, callback);
        };
      })(this));
      return async.waterfall(commands, callback);
    };

    Install.prototype.getPackageDependencies = function() {
      var error, metadata, packageDependencies, _ref;
      try {
        metadata = fs.readFileSync('package.json', 'utf8');
        packageDependencies = ((_ref = JSON.parse(metadata)) != null ? _ref : {}).packageDependencies;
        return packageDependencies != null ? packageDependencies : {};
      } catch (_error) {
        error = _error;
        return {};
      }
    };

    Install.prototype.createAtomDirectories = function() {
      fs.makeTreeSync(this.atomDirectory);
      fs.makeTreeSync(this.atomPackagesDirectory);
      return fs.makeTreeSync(this.atomNodeDirectory);
    };

    Install.prototype.checkNativeBuildTools = function(callback) {
      process.stdout.write('Checking for native build tools ');
      return this.installNode((function(_this) {
        return function(error) {
          var buildArgs, buildOptions, env, vsArgs;
          if (error != null) {
            _this.logFailure();
            return callback(error);
          }
          buildArgs = ['--globalconfig', config.getGlobalConfigPath(), '--userconfig', config.getUserConfigPath(), 'build'];
          buildArgs.push(path.resolve(__dirname, '..', 'native-module'));
          buildArgs.push("--target=" + (config.getNodeVersion()));
          buildArgs.push("--arch=" + (config.getNodeArch()));
          if (vsArgs = _this.getVisualStudioFlags()) {
            buildArgs.push(vsArgs);
          }
          env = _.extend({}, process.env, {
            HOME: _this.atomNodeDirectory
          });
          if (config.isWin32()) {
            _this.updateWindowsEnv(env);
          }
          _this.addNodeBinToEnv(env);
          _this.addProxyToEnv(env);
          buildOptions = {
            env: env
          };
          if (_this.verbose) {
            buildOptions.streaming = true;
          }
          fs.removeSync(path.resolve(__dirname, '..', 'native-module', 'build'));
          return _this.fork(_this.atomNpmPath, buildArgs, buildOptions, function() {
            var args;
            args = 1 <= arguments.length ? __slice.call(arguments, 0) : [];
            return _this.logCommandResults.apply(_this, [callback].concat(__slice.call(args)));
          });
        };
      })(this));
    };

    Install.prototype.packageNamesFromPath = function(filePath) {
      var packages;
      filePath = path.resolve(filePath);
      if (!fs.isFileSync(filePath)) {
        throw new Error("File '" + filePath + "' does not exist");
      }
      packages = fs.readFileSync(filePath, 'utf8');
      return this.sanitizePackageNames(packages.split(/\s/));
    };

    Install.prototype.getResourcePath = function(callback) {
      if (this.resourcePath) {
        return process.nextTick((function(_this) {
          return function() {
            return callback(_this.resourcePath);
          };
        })(this));
      } else {
        return config.getResourcePath((function(_this) {
          return function(resourcePath) {
            _this.resourcePath = resourcePath;
            return callback(_this.resourcePath);
          };
        })(this));
      }
    };

    Install.prototype.buildModuleCache = function(packageName, callback) {
      var packageDirectory, rebuildCacheCommand;
      packageDirectory = path.join(this.atomPackagesDirectory, packageName);
      rebuildCacheCommand = new RebuildModuleCache();
      return rebuildCacheCommand.rebuild(packageDirectory, function() {
        return callback();
      });
    };

    Install.prototype.warmCompileCache = function(packageName, callback) {
      var packageDirectory;
      packageDirectory = path.join(this.atomPackagesDirectory, packageName);
      return this.getResourcePath((function(_this) {
        return function(resourcePath) {
          var CompileCache, onDirectory, onFile;
          try {
            CompileCache = require(path.join(resourcePath, 'src', 'compile-cache'));
            onDirectory = function(directoryPath) {
              return path.basename(directoryPath) !== 'node_modules';
            };
            onFile = function(filePath) {
              try {
                return CompileCache.addPathToCache(filePath, _this.atomDirectory);
              } catch (_error) {}
            };
            fs.traverseTreeSync(packageDirectory, onFile, onDirectory);
          } catch (_error) {}
          return callback(null);
        };
      })(this));
    };

    Install.prototype.isBundledPackage = function(packageName, callback) {
      return this.getResourcePath(function(resourcePath) {
        var atomMetadata, error, _ref;
        try {
          atomMetadata = JSON.parse(fs.readFileSync(path.join(resourcePath, 'package.json')));
        } catch (_error) {
          error = _error;
          return callback(false);
        }
        return callback(atomMetadata != null ? (_ref = atomMetadata.packageDependencies) != null ? _ref.hasOwnProperty(packageName) : void 0 : void 0);
      });
    };

    Install.prototype.getLatestCompatibleVersion = function(pack) {
      var engine, latestVersion, metadata, version, _ref, _ref1, _ref2, _ref3;
      if (!this.installedAtomVersion) {
        return pack.releases.latest;
      }
      latestVersion = null;
      _ref1 = (_ref = pack.versions) != null ? _ref : {};
      for (version in _ref1) {
        metadata = _ref1[version];
        if (!semver.valid(version)) {
          continue;
        }
        if (!metadata) {
          continue;
        }
        engine = (_ref2 = (_ref3 = metadata.engines) != null ? _ref3.atom : void 0) != null ? _ref2 : '*';
        if (!semver.validRange(engine)) {
          continue;
        }
        if (!semver.satisfies(this.installedAtomVersion, engine)) {
          continue;
        }
        if (latestVersion == null) {
          latestVersion = version;
        }
        if (semver.gt(version, latestVersion)) {
          latestVersion = version;
        }
      }
      return latestVersion;
    };

    Install.prototype.loadInstalledAtomVersion = function(callback) {
      return this.getResourcePath((function(_this) {
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
    };

    Install.prototype.run = function(options) {
      var callback, commands, error, installPackage, packageNames, packagesFilePath;
      callback = options.callback;
      options = this.parseOptions(options.commandArgs);
      packagesFilePath = options.argv['packages-file'];
      this.createAtomDirectories();
      if (options.argv.check) {
        config.loadNpm((function(_this) {
          return function(error, npm) {
            _this.npm = npm;
            return _this.checkNativeBuildTools(callback);
          };
        })(this));
        return;
      }
      this.verbose = options.argv.verbose;
      if (this.verbose) {
        request.debug(true);
        process.env.NODE_DEBUG = 'request';
      }
      installPackage = (function(_this) {
        return function(name, callback) {
          var atIndex, version;
          if (name === '.') {
            return _this.installDependencies(options, callback);
          } else {
            atIndex = name.indexOf('@');
            if (atIndex > 0) {
              version = name.substring(atIndex + 1);
              name = name.substring(0, atIndex);
            }
            return _this.isBundledPackage(name, function(isBundledPackage) {
              if (isBundledPackage) {
                console.error(("The " + name + " package is bundled with Atom and should not be explicitly installed.\nYou can run `apm uninstall " + name + "` to uninstall it and then the version bundled\nwith Atom will be used.").yellow);
              }
              return _this.installPackage({
                name: name,
                version: version
              }, options, callback);
            });
          }
        };
      })(this);
      if (packagesFilePath) {
        try {
          packageNames = this.packageNamesFromPath(packagesFilePath);
        } catch (_error) {
          error = _error;
          return callback(error);
        }
      } else {
        packageNames = this.packageNamesFromArgv(options.argv);
        if (packageNames.length === 0) {
          packageNames.push('.');
        }
      }
      commands = [];
      commands.push((function(_this) {
        return function(callback) {
          return config.loadNpm(function(error, npm) {
            _this.npm = npm;
            return callback();
          });
        };
      })(this));
      commands.push((function(_this) {
        return function(callback) {
          return _this.loadInstalledAtomVersion(callback);
        };
      })(this));
      packageNames.forEach(function(packageName) {
        return commands.push(function(callback) {
          return installPackage(packageName, callback);
        });
      });
      return async.waterfall(commands, callback);
    };

    return Install;

  })(Command);

}).call(this);
