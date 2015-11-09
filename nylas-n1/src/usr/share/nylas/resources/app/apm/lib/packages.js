(function() {
  var url;

  url = require('url');

  module.exports = {
    getRepository: function(pack) {
      var name, owner, repoPath, repository, _ref, _ref1, _ref2;
      if (pack == null) {
        pack = {};
      }
      if (repository = (_ref = (_ref1 = pack.repository) != null ? _ref1.url : void 0) != null ? _ref : pack.repository) {
        repoPath = url.parse(repository.replace(/\.git$/, '')).pathname;
        _ref2 = repoPath.split('/').slice(-2), name = _ref2[0], owner = _ref2[1];
        if (name && owner) {
          return "" + name + "/" + owner;
        }
      }
      return null;
    }
  };

}).call(this);
