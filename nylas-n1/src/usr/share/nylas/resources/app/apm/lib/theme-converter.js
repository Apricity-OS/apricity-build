(function() {
  var TextMateTheme, ThemeConverter, fs, path, request, url;

  path = require('path');

  url = require('url');

  fs = require('./fs');

  request = require('./request');

  TextMateTheme = require('./text-mate-theme');

  module.exports = ThemeConverter = (function() {
    function ThemeConverter(sourcePath, destinationPath) {
      this.sourcePath = sourcePath;
      this.destinationPath = path.resolve(destinationPath);
    }

    ThemeConverter.prototype.readTheme = function(callback) {
      var protocol, requestOptions, sourcePath;
      protocol = url.parse(this.sourcePath).protocol;
      if (protocol === 'http:' || protocol === 'https:') {
        requestOptions = {
          url: this.sourcePath
        };
        return request.get(requestOptions, (function(_this) {
          return function(error, response, body) {
            if (error != null) {
              if (error.code === 'ENOTFOUND') {
                error = "Could not resolve URL: " + _this.sourcePath;
              }
              return callback(error);
            } else if (response.statusCode !== 200) {
              return callback("Request to " + _this.sourcePath + " failed (" + response.headers.status + ")");
            } else {
              return callback(null, body);
            }
          };
        })(this));
      } else {
        sourcePath = path.resolve(this.sourcePath);
        if (fs.isFileSync(sourcePath)) {
          return callback(null, fs.readFileSync(sourcePath, 'utf8'));
        } else {
          return callback("TextMate theme file not found: " + sourcePath);
        }
      }
    };

    ThemeConverter.prototype.convert = function(callback) {
      return this.readTheme((function(_this) {
        return function(error, themeContents) {
          var theme;
          if (error != null) {
            return callback(error);
          }
          try {
            theme = new TextMateTheme(themeContents);
          } catch (_error) {
            error = _error;
            return callback(error);
          }
          fs.writeFileSync(path.join(_this.destinationPath, 'styles', 'base.less'), theme.getStylesheet());
          fs.writeFileSync(path.join(_this.destinationPath, 'styles', 'syntax-variables.less'), theme.getSyntaxVariables());
          return callback();
        };
      })(this));
    };

    return ThemeConverter;

  })();

}).call(this);
