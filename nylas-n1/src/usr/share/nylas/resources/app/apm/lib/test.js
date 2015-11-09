(function() {
  var Command, Test, fs, optimist, path, temp,
    __hasProp = {}.hasOwnProperty,
    __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

  path = require('path');

  optimist = require('optimist');

  temp = require('temp');

  Command = require('./command');

  fs = require('./fs');

  module.exports = Test = (function(_super) {
    __extends(Test, _super);

    function Test() {
      return Test.__super__.constructor.apply(this, arguments);
    }

    Test.commandNames = ['test'];

    Test.prototype.parseOptions = function(argv) {
      var options;
      options = optimist(argv);
      options.usage("Usage:\n  apm test\n\nRuns the package's tests contained within the spec directory (relative\nto the current working directory).");
      options.alias('h', 'help').describe('help', 'Print this usage message');
      options.alias('1', 'one').boolean('one').describe('one', 'Run specs in 1.0 API preview mode');
      return options.alias('p', 'path').string('path').describe('path', 'Path to atom command');
    };

    Test.prototype.run = function(options) {
      var atomCommand, callback, env, logFile, logFilePath, packagePath, testArgs;
      callback = options.callback;
      options = this.parseOptions(options.commandArgs);
      env = process.env;
      if (options.argv.path) {
        atomCommand = options.argv.path;
      }
      if (!fs.existsSync(atomCommand)) {
        atomCommand = 'atom';
        if (process.platform === 'win32') {
          atomCommand += '.cmd';
        }
      }
      packagePath = process.cwd();
      testArgs = ['--dev', '--test', "--spec-directory=" + (path.join(packagePath, 'spec'))];
      if (options.argv.one) {
        testArgs.push('--one');
      }
      if (process.platform === 'win32') {
        logFile = temp.openSync({
          suffix: '.log',
          prefix: "" + (path.basename(packagePath)) + "-"
        });
        fs.closeSync(logFile.fd);
        logFilePath = logFile.path;
        testArgs.push("--log-file=" + logFilePath);
        return this.spawn(atomCommand, testArgs, function(code) {
          var loggedOutput;
          try {
            loggedOutput = fs.readFileSync(logFilePath, 'utf8');
            if (loggedOutput) {
              process.stdout.write("" + loggedOutput + "\n");
            }
          } catch (_error) {}
          if (code === 0) {
            process.stdout.write('Tests passed\n'.green);
            return callback();
          } else if (code != null ? code.message : void 0) {
            return callback("Error spawning Atom: " + code.message);
          } else {
            return callback('Tests failed');
          }
        });
      } else {
        return this.spawn(atomCommand, testArgs, {
          env: env,
          streaming: true
        }, function(code) {
          if (code === 0) {
            process.stdout.write('Tests passed\n'.green);
            return callback();
          } else if (code != null ? code.message : void 0) {
            return callback("Error spawning " + atomCommand + ": " + code.message);
          } else {
            return callback('Tests failed');
          }
        });
      }
    };

    return Test;

  })(Command);

}).call(this);
