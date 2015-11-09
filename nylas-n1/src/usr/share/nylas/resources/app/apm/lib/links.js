(function() {
  var Command, Links, config, fs, optimist, path, tree,
    __hasProp = {}.hasOwnProperty,
    __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

  path = require('path');

  optimist = require('optimist');

  Command = require('./command');

  config = require('./apm');

  fs = require('./fs');

  tree = require('./tree');

  module.exports = Links = (function(_super) {
    __extends(Links, _super);

    Links.commandNames = ['linked', 'links', 'lns'];

    function Links() {
      this.devPackagesPath = path.join(config.getAtomDirectory(), 'dev', 'packages');
      this.packagesPath = path.join(config.getAtomDirectory(), 'packages');
    }

    Links.prototype.parseOptions = function(argv) {
      var options;
      options = optimist(argv);
      options.usage("\nUsage: apm links\n\nList all of the symlinked atom packages in ~/.atom/packages and\n~/.atom/dev/packages.");
      return options.alias('h', 'help').describe('help', 'Print this usage message');
    };

    Links.prototype.getDevPackagePath = function(packageName) {
      return path.join(this.devPackagesPath, packageName);
    };

    Links.prototype.getPackagePath = function(packageName) {
      return path.join(this.packagesPath, packageName);
    };

    Links.prototype.getSymlinks = function(directoryPath) {
      var directory, symlinkPath, symlinks, _i, _len, _ref;
      symlinks = [];
      _ref = fs.list(directoryPath);
      for (_i = 0, _len = _ref.length; _i < _len; _i++) {
        directory = _ref[_i];
        symlinkPath = path.join(directoryPath, directory);
        if (fs.isSymbolicLinkSync(symlinkPath)) {
          symlinks.push(symlinkPath);
        }
      }
      return symlinks;
    };

    Links.prototype.logLinks = function(directoryPath) {
      var links;
      links = this.getSymlinks(directoryPath);
      console.log("" + directoryPath.cyan + " (" + links.length + ")");
      return tree(links, {
        emptyMessage: '(no links)'
      }, (function(_this) {
        return function(link) {
          var error, realpath;
          try {
            realpath = fs.realpathSync(link);
          } catch (_error) {
            error = _error;
            realpath = '???'.red;
          }
          return "" + (path.basename(link).yellow) + " -> " + realpath;
        };
      })(this));
    };

    Links.prototype.run = function(options) {
      var callback;
      callback = options.callback;
      this.logLinks(this.devPackagesPath);
      this.logLinks(this.packagesPath);
      return callback();
    };

    return Links;

  })(Command);

}).call(this);
