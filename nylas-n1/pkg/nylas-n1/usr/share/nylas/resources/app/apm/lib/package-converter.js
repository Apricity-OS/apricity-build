(function() {
  var CSON, PackageConverter, ScopeSelector, fs, path, plist, request, tar, temp, url, zlib, _;

  path = require('path');

  url = require('url');

  zlib = require('zlib');

  _ = require('underscore-plus');

  CSON = require('season');

  plist = require('plist');

  ScopeSelector = require('first-mate').ScopeSelector;

  tar = require('npm/node_modules/tar');

  temp = require('temp');

  fs = require('./fs');

  request = require('./request');

  module.exports = PackageConverter = (function() {
    function PackageConverter(sourcePath, destinationPath) {
      this.sourcePath = sourcePath;
      this.destinationPath = path.resolve(destinationPath);
      this.plistExtensions = ['.plist', '.tmCommand', '.tmLanguage', '.tmMacro', '.tmPreferences', '.tmSnippet'];
      this.directoryMappings = {
        'Preferences': 'settings',
        'Snippets': 'snippets',
        'Syntaxes': 'grammars'
      };
    }

    PackageConverter.prototype.convert = function(callback) {
      var protocol;
      protocol = url.parse(this.sourcePath).protocol;
      if (protocol === 'http:' || protocol === 'https:') {
        return this.downloadBundle(callback);
      } else {
        return this.copyDirectories(this.sourcePath, callback);
      }
    };

    PackageConverter.prototype.getDownloadUrl = function() {
      var downloadUrl;
      downloadUrl = this.sourcePath;
      downloadUrl = downloadUrl.replace(/(\.git)?\/*$/, '');
      return downloadUrl += '/archive/master.tar.gz';
    };

    PackageConverter.prototype.downloadBundle = function(callback) {
      var requestOptions, tempPath;
      tempPath = temp.mkdirSync('atom-bundle-');
      requestOptions = {
        url: this.getDownloadUrl()
      };
      return request.createReadStream(requestOptions, (function(_this) {
        return function(readStream) {
          readStream.on('response', function(_arg) {
            var headers, statusCode;
            headers = _arg.headers, statusCode = _arg.statusCode;
            if (statusCode !== 200) {
              return callback("Download failed (" + headers.status + ")");
            }
          });
          return readStream.pipe(zlib.createGunzip()).pipe(tar.Extract({
            path: tempPath
          })).on('error', function(error) {
            return callback(error);
          }).on('end', function() {
            var sourcePath;
            sourcePath = path.join(tempPath, fs.readdirSync(tempPath)[0]);
            return _this.copyDirectories(sourcePath, callback);
          });
        };
      })(this));
    };

    PackageConverter.prototype.copyDirectories = function(sourcePath, callback) {
      var packageName, _ref;
      sourcePath = path.resolve(sourcePath);
      try {
        packageName = (_ref = JSON.parse(fs.readFileSync(path.join(sourcePath, 'package.json')))) != null ? _ref.packageName : void 0;
      } catch (_error) {}
      if (packageName == null) {
        packageName = path.basename(this.destinationPath);
      }
      this.convertSnippets(packageName, sourcePath);
      this.convertPreferences(packageName, sourcePath);
      this.convertGrammars(sourcePath);
      return callback();
    };

    PackageConverter.prototype.filterObject = function(object) {
      delete object.uuid;
      return delete object.keyEquivalent;
    };

    PackageConverter.prototype.convertSettings = function(settings) {
      var editorProperties, name, shellVariables, value, _i, _len, _ref, _ref1;
      if (settings.shellVariables) {
        shellVariables = {};
        _ref = settings.shellVariables;
        for (_i = 0, _len = _ref.length; _i < _len; _i++) {
          _ref1 = _ref[_i], name = _ref1.name, value = _ref1.value;
          shellVariables[name] = value;
        }
        settings.shellVariables = shellVariables;
      }
      editorProperties = _.compactObject({
        commentStart: _.valueForKeyPath(settings, 'shellVariables.TM_COMMENT_START'),
        commentEnd: _.valueForKeyPath(settings, 'shellVariables.TM_COMMENT_END'),
        increaseIndentPattern: settings.increaseIndentPattern,
        decreaseIndentPattern: settings.decreaseIndentPattern,
        foldEndPattern: settings.foldingStopMarker,
        completions: settings.completions
      });
      if (!_.isEmpty(editorProperties)) {
        return {
          editor: editorProperties
        };
      }
    };

    PackageConverter.prototype.readFileSync = function(filePath) {
      if (_.contains(this.plistExtensions, path.extname(filePath))) {
        return plist.parseFileSync(filePath);
      } else if (_.contains(['.json', '.cson'], path.extname(filePath))) {
        return CSON.readFileSync(filePath);
      }
    };

    PackageConverter.prototype.writeFileSync = function(filePath, object) {
      if (object == null) {
        object = {};
      }
      this.filterObject(object);
      if (Object.keys(object).length > 0) {
        return CSON.writeFileSync(filePath, object);
      }
    };

    PackageConverter.prototype.convertFile = function(sourcePath, destinationDir) {
      var contents, destinationName, destinationPath, extension;
      extension = path.extname(sourcePath);
      destinationName = "" + (path.basename(sourcePath, extension)) + ".cson";
      destinationName = destinationName.toLowerCase();
      destinationPath = path.join(destinationDir, destinationName);
      if (_.contains(this.plistExtensions, path.extname(sourcePath))) {
        contents = plist.parseFileSync(sourcePath);
      } else if (_.contains(['.json', '.cson'], path.extname(sourcePath))) {
        contents = CSON.readFileSync(sourcePath);
      }
      return this.writeFileSync(destinationPath, contents);
    };

    PackageConverter.prototype.normalizeFilenames = function(directoryPath) {
      var child, childPath, convertedFileName, convertedPath, extension, suffix, _i, _len, _ref, _results;
      if (!fs.isDirectorySync(directoryPath)) {
        return;
      }
      _ref = fs.readdirSync(directoryPath);
      _results = [];
      for (_i = 0, _len = _ref.length; _i < _len; _i++) {
        child = _ref[_i];
        childPath = path.join(directoryPath, child);
        convertedFileName = child.replace(/[|?*<>:"\\\/]+/g, '-');
        if (child === convertedFileName) {
          continue;
        }
        convertedFileName = convertedFileName.replace(/[\s-]+/g, '-');
        convertedPath = path.join(directoryPath, convertedFileName);
        suffix = 1;
        while (fs.existsSync(convertedPath) || fs.existsSync(convertedPath.toLowerCase())) {
          extension = path.extname(convertedFileName);
          convertedFileName = "" + (path.basename(convertedFileName, extension)) + "-" + suffix + extension;
          convertedPath = path.join(directoryPath, convertedFileName);
          suffix++;
        }
        _results.push(fs.renameSync(childPath, convertedPath));
      }
      return _results;
    };

    PackageConverter.prototype.convertSnippets = function(packageName, source) {
      var child, content, destination, extension, name, scope, selector, snippet, snippetsBySelector, sourceSnippets, tabTrigger, _i, _len, _ref, _ref1;
      sourceSnippets = path.join(source, 'snippets');
      if (!fs.isDirectorySync(sourceSnippets)) {
        sourceSnippets = path.join(source, 'Snippets');
      }
      if (!fs.isDirectorySync(sourceSnippets)) {
        return;
      }
      snippetsBySelector = {};
      destination = path.join(this.destinationPath, 'snippets');
      _ref = fs.readdirSync(sourceSnippets);
      for (_i = 0, _len = _ref.length; _i < _len; _i++) {
        child = _ref[_i];
        snippet = (_ref1 = this.readFileSync(path.join(sourceSnippets, child))) != null ? _ref1 : {};
        scope = snippet.scope, name = snippet.name, content = snippet.content, tabTrigger = snippet.tabTrigger;
        if (!(tabTrigger && content)) {
          continue;
        }
        content = content.replace(/\$\{TM_[A-Z_]+:([^}]+)}/g, '$1');
        content = content.replace(/\$\{(\d)+:\s*\$\{TM_[^}]+\s*\}\s*\}/g, '$$1');
        content = content.replace(/\\\$/g, '$');
        if (name == null) {
          extension = path.extname(child);
          name = path.basename(child, extension);
        }
        if (scope) {
          selector = new ScopeSelector(scope).toCssSelector();
        }
        if (selector == null) {
          selector = '*';
        }
        if (snippetsBySelector[selector] == null) {
          snippetsBySelector[selector] = {};
        }
        snippetsBySelector[selector][name] = {
          prefix: tabTrigger,
          body: content
        };
      }
      this.writeFileSync(path.join(destination, "" + packageName + ".cson"), snippetsBySelector);
      return this.normalizeFilenames(destination);
    };

    PackageConverter.prototype.convertPreferences = function(packageName, source) {
      var child, destination, key, preferencesBySelector, properties, scope, selector, settings, sourcePreferences, value, _i, _len, _ref, _ref1, _ref2;
      sourcePreferences = path.join(source, 'preferences');
      if (!fs.isDirectorySync(sourcePreferences)) {
        sourcePreferences = path.join(source, 'Preferences');
      }
      if (!fs.isDirectorySync(sourcePreferences)) {
        return;
      }
      preferencesBySelector = {};
      destination = path.join(this.destinationPath, 'settings');
      _ref = fs.readdirSync(sourcePreferences);
      for (_i = 0, _len = _ref.length; _i < _len; _i++) {
        child = _ref[_i];
        _ref2 = (_ref1 = this.readFileSync(path.join(sourcePreferences, child))) != null ? _ref1 : {}, scope = _ref2.scope, settings = _ref2.settings;
        if (!(scope && settings)) {
          continue;
        }
        if (properties = this.convertSettings(settings)) {
          selector = new ScopeSelector(scope).toCssSelector();
          for (key in properties) {
            value = properties[key];
            if (preferencesBySelector[selector] == null) {
              preferencesBySelector[selector] = {};
            }
            if (preferencesBySelector[selector][key] != null) {
              preferencesBySelector[selector][key] = _.extend(value, preferencesBySelector[selector][key]);
            } else {
              preferencesBySelector[selector][key] = value;
            }
          }
        }
      }
      this.writeFileSync(path.join(destination, "" + packageName + ".cson"), preferencesBySelector);
      return this.normalizeFilenames(destination);
    };

    PackageConverter.prototype.convertGrammars = function(source) {
      var child, childPath, destination, sourceSyntaxes, _i, _len, _ref;
      sourceSyntaxes = path.join(source, 'syntaxes');
      if (!fs.isDirectorySync(sourceSyntaxes)) {
        sourceSyntaxes = path.join(source, 'Syntaxes');
      }
      if (!fs.isDirectorySync(sourceSyntaxes)) {
        return;
      }
      destination = path.join(this.destinationPath, 'grammars');
      _ref = fs.readdirSync(sourceSyntaxes);
      for (_i = 0, _len = _ref.length; _i < _len; _i++) {
        child = _ref[_i];
        childPath = path.join(sourceSyntaxes, child);
        if (fs.isFileSync(childPath)) {
          this.convertFile(childPath, destination);
        }
      }
      return this.normalizeFilenames(destination);
    };

    return PackageConverter;

  })();

}).call(this);
