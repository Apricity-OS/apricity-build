(function() {
  var Docs, View, config, open, optimist,
    __hasProp = {}.hasOwnProperty,
    __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

  optimist = require('optimist');

  open = require('open');

  View = require('./view');

  config = require('./apm');

  module.exports = Docs = (function(_super) {
    __extends(Docs, _super);

    function Docs() {
      return Docs.__super__.constructor.apply(this, arguments);
    }

    Docs.commandNames = ['docs', 'home', 'open'];

    Docs.prototype.parseOptions = function(argv) {
      var options;
      options = optimist(argv);
      options.usage("\nUsage: apm docs [options] <package_name>\n\nOpen a package's homepage in the default browser.");
      options.alias('h', 'help').describe('help', 'Print this usage message');
      return options.boolean('p').alias('p', 'print').describe('print', 'Print the URL instead of opening it');
    };

    Docs.prototype.openRepositoryUrl = function(repositoryUrl) {
      return open(repositoryUrl);
    };

    Docs.prototype.run = function(options) {
      var callback, packageName;
      callback = options.callback;
      options = this.parseOptions(options.commandArgs);
      packageName = options.argv._[0];
      if (!packageName) {
        callback("Missing required package name");
        return;
      }
      return this.getPackage(packageName, options, (function(_this) {
        return function(error, pack) {
          var repository;
          if (error != null) {
            return callback(error);
          }
          if (repository = _this.getRepository(pack)) {
            if (options.argv.print) {
              console.log(repository);
            } else {
              _this.openRepositoryUrl(repository);
            }
            return callback();
          } else {
            return callback("Package \"" + packageName + "\" does not contain a repository URL");
          }
        };
      })(this));
    };

    return Docs;

  })(View);

}).call(this);
