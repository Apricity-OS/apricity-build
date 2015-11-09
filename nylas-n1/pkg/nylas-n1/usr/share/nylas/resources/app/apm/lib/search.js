(function() {
  var Command, Search, config, optimist, request, tree, _,
    __hasProp = {}.hasOwnProperty,
    __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

  _ = require('underscore-plus');

  optimist = require('optimist');

  Command = require('./command');

  config = require('./apm');

  request = require('./request');

  tree = require('./tree');

  module.exports = Search = (function(_super) {
    __extends(Search, _super);

    function Search() {
      return Search.__super__.constructor.apply(this, arguments);
    }

    Search.commandNames = ['search'];

    Search.prototype.parseOptions = function(argv) {
      var options;
      options = optimist(argv);
      options.usage("\nUsage: apm search <package_name>\n\nSearch for Atom packages/themes on the atom.io registry.");
      options.alias('h', 'help').describe('help', 'Print this usage message');
      options.boolean('json').describe('json', 'Output matching packages as JSON array');
      options.boolean('packages').describe('packages', 'Search only non-theme packages').alias('p', 'packages');
      return options.boolean('themes').describe('themes', 'Search only themes').alias('t', 'themes');
    };

    Search.prototype.searchPackages = function(query, opts, callback) {
      var qs, requestSettings;
      qs = {
        q: query
      };
      if (opts.packages) {
        qs.filter = 'package';
      } else if (opts.themes) {
        qs.filter = 'theme';
      }
      requestSettings = {
        url: "" + (config.getAtomPackagesUrl()) + "/search",
        qs: qs,
        json: true
      };
      return request.get(requestSettings, function(error, response, body) {
        var message, packages;
        if (body == null) {
          body = {};
        }
        if (error != null) {
          return callback(error);
        } else if (response.statusCode === 200) {
          packages = body.filter(function(pack) {
            var _ref;
            return ((_ref = pack.releases) != null ? _ref.latest : void 0) != null;
          });
          packages = packages.map(function(_arg) {
            var downloads, metadata, readme, stargazers_count;
            readme = _arg.readme, metadata = _arg.metadata, downloads = _arg.downloads, stargazers_count = _arg.stargazers_count;
            return _.extend({}, metadata, {
              readme: readme,
              downloads: downloads,
              stargazers_count: stargazers_count
            });
          });
          return callback(null, packages);
        } else {
          message = request.getErrorMessage(response, body);
          return callback("Searching packages failed: " + message);
        }
      });
    };

    Search.prototype.run = function(options) {
      var callback, query, searchOptions;
      callback = options.callback;
      options = this.parseOptions(options.commandArgs);
      query = options.argv._[0];
      if (!query) {
        callback("Missing required search query");
        return;
      }
      searchOptions = {
        packages: options.argv.packages,
        themes: options.argv.themes
      };
      return this.searchPackages(query, searchOptions, function(error, packages) {
        var heading;
        if (error != null) {
          callback(error);
          return;
        }
        if (options.argv.json) {
          console.log(JSON.stringify(packages));
        } else {
          heading = ("Search Results For '" + query + "'").cyan;
          console.log("" + heading + " (" + packages.length + ")");
          tree(packages, function(_arg) {
            var description, downloads, label, name, stargazers_count, version;
            name = _arg.name, version = _arg.version, description = _arg.description, downloads = _arg.downloads, stargazers_count = _arg.stargazers_count;
            label = name.yellow;
            if (description) {
              label += " " + (description.replace(/\s+/g, ' '));
            }
            if (downloads >= 0 && stargazers_count >= 0) {
              label += (" (" + (_.pluralize(downloads, 'download')) + ", " + (_.pluralize(stargazers_count, 'star')) + ")").grey;
            }
            return label;
          });
          console.log();
          console.log("Use `apm install` to install them or visit " + 'http://atom.io/packages'.underline + " to read more about them.");
          console.log();
        }
        return callback();
      });
    };

    return Search;

  })(Command);

}).call(this);
