(function() {
  var fs, fsAdditions, ncp, rm, wrench, _;

  _ = require('underscore-plus');

  fs = require('fs-plus');

  ncp = require('ncp');

  rm = require('npm/node_modules/rimraf');

  wrench = require('wrench');

  fsAdditions = {
    list: function(directoryPath) {
      var e;
      if (fs.isDirectorySync(directoryPath)) {
        try {
          return fs.readdirSync(directoryPath);
        } catch (_error) {
          e = _error;
          return [];
        }
      } else {
        return [];
      }
    },
    listRecursive: function(directoryPath) {
      return wrench.readdirSyncRecursive(directoryPath);
    },
    cp: function(sourcePath, destinationPath, callback) {
      return rm(destinationPath, function(error) {
        if (error != null) {
          return callback(error);
        } else {
          return ncp(sourcePath, destinationPath, callback);
        }
      });
    }
  };

  module.exports = _.extend({}, fs, fsAdditions);

}).call(this);
