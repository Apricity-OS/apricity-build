(function() {
  var Command, child_process, _,
    __bind = function(fn, me){ return function(){ return fn.apply(me, arguments); }; },
    __slice = [].slice;

  child_process = require('child_process');

  _ = require('underscore-plus');

  module.exports = Command = (function() {
    function Command() {
      this.logCommandResults = __bind(this.logCommandResults, this);
      this.logFailure = __bind(this.logFailure, this);
      this.logSuccess = __bind(this.logSuccess, this);
    }

    Command.prototype.spawn = function() {
      var args, callback, command, errorChunks, options, outputChunks, remaining, spawned;
      command = arguments[0], args = arguments[1], remaining = 3 <= arguments.length ? __slice.call(arguments, 2) : [];
      if (remaining.length >= 2) {
        options = remaining.shift();
      }
      callback = remaining.shift();
      spawned = child_process.spawn(command, args, options);
      errorChunks = [];
      outputChunks = [];
      spawned.stdout.on('data', function(chunk) {
        if (options != null ? options.streaming : void 0) {
          return process.stdout.write(chunk);
        } else {
          return outputChunks.push(chunk);
        }
      });
      spawned.stderr.on('data', function(chunk) {
        if (options != null ? options.streaming : void 0) {
          return process.stderr.write(chunk);
        } else {
          return errorChunks.push(chunk);
        }
      });
      spawned.on('error', function(error) {
        return callback(error, Buffer.concat(errorChunks).toString(), Buffer.concat(outputChunks).toString());
      });
      return spawned.on('close', function(code) {
        return callback(code, Buffer.concat(errorChunks).toString(), Buffer.concat(outputChunks).toString());
      });
    };

    Command.prototype.fork = function() {
      var args, remaining, script;
      script = arguments[0], args = arguments[1], remaining = 3 <= arguments.length ? __slice.call(arguments, 2) : [];
      args.unshift(script);
      return this.spawn.apply(this, [process.execPath, args].concat(__slice.call(remaining)));
    };

    Command.prototype.packageNamesFromArgv = function(argv) {
      return this.sanitizePackageNames(argv._);
    };

    Command.prototype.sanitizePackageNames = function(packageNames) {
      if (packageNames == null) {
        packageNames = [];
      }
      packageNames = packageNames.map(function(packageName) {
        return packageName.trim();
      });
      return _.compact(_.uniq(packageNames));
    };

    Command.prototype.logSuccess = function() {
      if (process.platform === 'win32') {
        return process.stdout.write('done\n'.green);
      } else {
        return process.stdout.write('\u2713\n'.green);
      }
    };

    Command.prototype.logFailure = function() {
      if (process.platform === 'win32') {
        return process.stdout.write('failed\n'.red);
      } else {
        return process.stdout.write('\u2717\n'.red);
      }
    };

    Command.prototype.logCommandResults = function(callback, code, stderr, stdout) {
      if (stderr == null) {
        stderr = '';
      }
      if (stdout == null) {
        stdout = '';
      }
      if (code === 0) {
        this.logSuccess();
        return callback();
      } else {
        this.logFailure();
        return callback(("" + stdout + "\n" + stderr).trim());
      }
    };

    Command.prototype.normalizeVersion = function(version) {
      if (typeof version === 'string') {
        return version.replace(/-.*$/, '');
      } else {
        return version;
      }
    };

    return Command;

  })();

}).call(this);
