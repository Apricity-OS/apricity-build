(function() {
  var _;

  _ = require('underscore-plus');

  module.exports = function(items, options, callback) {
    var emptyMessage, index, item, itemLine, _i, _len, _ref, _results;
    if (options == null) {
      options = {};
    }
    if (_.isFunction(options)) {
      callback = options;
      options = {};
    }
    if (callback == null) {
      callback = function(item) {
        return item;
      };
    }
    if (items.length === 0) {
      emptyMessage = (_ref = options.emptyMessage) != null ? _ref : '(empty)';
      return console.log("\u2514\u2500\u2500 " + emptyMessage);
    } else {
      _results = [];
      for (index = _i = 0, _len = items.length; _i < _len; index = ++_i) {
        item = items[index];
        if (index === items.length - 1) {
          itemLine = '\u2514\u2500\u2500 ';
        } else {
          itemLine = '\u251C\u2500\u2500 ';
        }
        _results.push(console.log("" + itemLine + (callback(item))));
      }
      return _results;
    }
  };

}).call(this);
