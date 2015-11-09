(function() {
  var colors, commandClass, commandClasses, commands, config, fs, getPythonVersion, git, name, npm, optimist, parseOptions, path, printVersions, setupTempDirectory, showHelp, spawn, wordwrap, _, _i, _j, _len, _len1, _ref, _ref1;

  spawn = require('child_process').spawn;

  path = require('path');

  _ = require('underscore-plus');

  colors = require('colors');

  npm = require('npm');

  optimist = require('optimist');

  wordwrap = require('wordwrap');

  require('asar-require');

  config = require('./apm');

  fs = require('./fs');

  git = require('./git');

  setupTempDirectory = function() {
    var temp, tempDirectory;
    temp = require('temp');
    tempDirectory = require('os').tmpdir();
    tempDirectory = path.resolve(fs.absolute(tempDirectory));
    temp.dir = tempDirectory;
    try {
      fs.makeTreeSync(temp.dir);
    } catch (_error) {}
    return temp.track();
  };

  setupTempDirectory();

  commandClasses = [require('./clean'), require('./config'), require('./dedupe'), require('./develop'), require('./docs'), require('./featured'), require('./init'), require('./install'), require('./links'), require('./link'), require('./list'), require('./login'), require('./publish'), require('./rebuild'), require('./rebuild-module-cache'), require('./search'), require('./star'), require('./stars'), require('./test'), require('./uninstall'), require('./unlink'), require('./unpublish'), require('./unstar'), require('./upgrade'), require('./view')];

  commands = {};

  for (_i = 0, _len = commandClasses.length; _i < _len; _i++) {
    commandClass = commandClasses[_i];
    _ref1 = (_ref = commandClass.commandNames) != null ? _ref : [];
    for (_j = 0, _len1 = _ref1.length; _j < _len1; _j++) {
      name = _ref1[_j];
      commands[name] = commandClass;
    }
  }

  parseOptions = function(args) {
    var arg, index, options, _k, _len2;
    if (args == null) {
      args = [];
    }
    options = optimist(args);
    options.usage("\napm - Atom Package Manager powered by https://atom.io\n\nUsage: apm <command>\n\nwhere <command> is one of:\n" + (wordwrap(4, 80)(Object.keys(commands).sort().join(', '))) + ".\n\nRun `apm help <command>` to see the more details about a specific command.");
    options.alias('v', 'version').describe('version', 'Print the apm version');
    options.alias('h', 'help').describe('help', 'Print this usage message');
    options.boolean('color')["default"]('color', true).describe('color', 'Enable colored output');
    options.command = options.argv._[0];
    for (index = _k = 0, _len2 = args.length; _k < _len2; index = ++_k) {
      arg = args[index];
      if (!(arg === options.command)) {
        continue;
      }
      options.commandArgs = args.slice(index + 1);
      break;
    }
    return options;
  };

  showHelp = function(options) {
    var help;
    if (options == null) {
      return;
    }
    help = options.help();
    if (help.indexOf('Options:') >= 0) {
      help += "\n  Prefix an option with `no-` to set it to false such as --no-color to disable";
      help += "\n  colored output.";
    }
    return console.error(help);
  };

  printVersions = function(args, callback) {
    var apmVersion, nodeVersion, npmVersion, _ref2, _ref3, _ref4;
    apmVersion = (_ref2 = require('../package.json').version) != null ? _ref2 : '';
    npmVersion = (_ref3 = require('npm/package.json').version) != null ? _ref3 : '';
    nodeVersion = (_ref4 = process.versions.node) != null ? _ref4 : '';
    return getPythonVersion(function(pythonVersion) {
      return git.getGitVersion(function(gitVersion) {
        var versions, visualStudioVersion, _ref5;
        if (args.json) {
          versions = {
            apm: apmVersion,
            npm: npmVersion,
            node: nodeVersion,
            python: pythonVersion,
            git: gitVersion
          };
          if (config.isWin32()) {
            versions.visualStudio = config.getInstalledVisualStudioFlag();
          }
          console.log(JSON.stringify(versions));
        } else {
          if (pythonVersion == null) {
            pythonVersion = '';
          }
          if (gitVersion == null) {
            gitVersion = '';
          }
          versions = "" + 'apm'.red + "  " + apmVersion.red + "\n" + 'npm'.green + "  " + npmVersion.green + "\n" + 'node'.blue + " " + nodeVersion.blue + "\n" + 'python'.yellow + " " + pythonVersion.yellow + "\n" + 'git'.magenta + " " + gitVersion.magenta;
          if (config.isWin32()) {
            visualStudioVersion = (_ref5 = config.getInstalledVisualStudioFlag()) != null ? _ref5 : '';
            versions += "\n" + 'visual studio'.cyan + " " + visualStudioVersion.cyan;
          }
          console.log(versions);
        }
        return callback();
      });
    });
  };

  getPythonVersion = function(callback) {
    var npmOptions;
    npmOptions = {
      userconfig: config.getUserConfigPath(),
      globalconfig: config.getGlobalConfigPath()
    };
    return npm.load(npmOptions, function() {
      var outputChunks, python, pythonExe, rootDir, spawned, _ref2, _ref3;
      python = (_ref2 = npm.config.get('python')) != null ? _ref2 : process.env.PYTHON;
      if (config.isWin32() && !python) {
        rootDir = (_ref3 = process.env.SystemDrive) != null ? _ref3 : 'C:\\';
        if (rootDir[rootDir.length - 1] !== '\\') {
          rootDir += '\\';
        }
        pythonExe = path.resolve(rootDir, 'Python27', 'python.exe');
        if (fs.isFileSync(pythonExe)) {
          python = pythonExe;
        }
      }
      if (python == null) {
        python = 'python';
      }
      spawned = spawn(python, ['--version']);
      outputChunks = [];
      spawned.stderr.on('data', function(chunk) {
        return outputChunks.push(chunk);
      });
      spawned.stdout.on('data', function(chunk) {
        return outputChunks.push(chunk);
      });
      spawned.on('error', function() {});
      return spawned.on('close', function(code) {
        var version, _ref4;
        if (code === 0) {
          _ref4 = Buffer.concat(outputChunks).toString().split(' '), name = _ref4[0], version = _ref4[1];
          version = version != null ? version.trim() : void 0;
        }
        return callback(version);
      });
    });
  };

  module.exports = {
    run: function(args, callback) {
      var Command, callbackCalled, command, options, _base, _base1;
      config.setupApmRcFile();
      options = parseOptions(args);
      if (!options.argv.color) {
        colors.setTheme({
          blue: 'stripColors',
          cyan: 'stripColors',
          green: 'stripColors',
          magenta: 'stripColors',
          red: 'stripColors',
          yellow: 'stripColors',
          rainbow: 'stripColors'
        });
      }
      callbackCalled = false;
      options.callback = function(error) {
        var message, _ref2;
        if (callbackCalled) {
          return;
        }
        callbackCalled = true;
        if (error != null) {
          if (_.isString(error)) {
            message = error;
          } else {
            message = (_ref2 = error.message) != null ? _ref2 : error;
          }
          if (message === 'canceled') {
            console.log();
          } else if (message) {
            console.error(message.red);
          }
        }
        return typeof callback === "function" ? callback(error) : void 0;
      };
      args = options.argv;
      command = options.command;
      if (args.version) {
        return printVersions(args, options.callback);
      } else if (args.help) {
        if (Command = commands[options.command]) {
          showHelp(typeof (_base = new Command()).parseOptions === "function" ? _base.parseOptions(options.command) : void 0);
        } else {
          showHelp(options);
        }
        return options.callback();
      } else if (command) {
        if (command === 'help') {
          if (Command = commands[options.commandArgs]) {
            showHelp(typeof (_base1 = new Command()).parseOptions === "function" ? _base1.parseOptions(options.commandArgs) : void 0);
          } else {
            showHelp(options);
          }
          return options.callback();
        } else if (Command = commands[command]) {
          return new Command().run(options);
        } else {
          return options.callback("Unrecognized command: " + command);
        }
      } else {
        showHelp(options);
        return options.callback();
      }
    }
  };

}).call(this);
