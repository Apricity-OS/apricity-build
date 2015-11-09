(function() {
  var addGitBashToEnv, addPortableGitToEnv, config, fs, npm, path, spawn, _;

  spawn = require('child_process').spawn;

  path = require('path');

  _ = require('underscore-plus');

  npm = require('npm');

  config = require('./apm');

  fs = require('./fs');

  addPortableGitToEnv = function(env) {
    var binPath, child, children, cmdPath, error, githubPath, localAppData, _i, _len;
    localAppData = env.LOCALAPPDATA;
    if (!localAppData) {
      return;
    }
    githubPath = path.join(localAppData, 'GitHub');
    try {
      children = fs.readdirSync(githubPath);
    } catch (_error) {
      error = _error;
      return;
    }
    for (_i = 0, _len = children.length; _i < _len; _i++) {
      child = children[_i];
      if (!(child.indexOf('PortableGit_') === 0)) {
        continue;
      }
      cmdPath = path.join(githubPath, child, 'cmd');
      binPath = path.join(githubPath, child, 'bin');
      if (env.Path) {
        env.Path += "" + path.delimiter + cmdPath + path.delimiter + binPath;
      } else {
        env.Path = "" + cmdPath + path.delimiter + binPath;
      }
      break;
    }
  };

  addGitBashToEnv = function(env) {
    var binPath, cmdPath, gitPath;
    if (env.ProgramFiles) {
      gitPath = path.join(env.ProgramFiles, 'Git');
    }
    if (!fs.isDirectorySync(gitPath)) {
      if (env['ProgramFiles(x86)']) {
        gitPath = path.join(env['ProgramFiles(x86)'], 'Git');
      }
    }
    if (!fs.isDirectorySync(gitPath)) {
      return;
    }
    cmdPath = path.join(gitPath, 'cmd');
    binPath = path.join(gitPath, 'bin');
    if (env.Path) {
      return env.Path += "" + path.delimiter + cmdPath + path.delimiter + binPath;
    } else {
      return env.Path = "" + cmdPath + path.delimiter + binPath;
    }
  };

  exports.addGitToEnv = function(env) {
    if (process.platform !== 'win32') {
      return;
    }
    addPortableGitToEnv(env);
    return addGitBashToEnv(env);
  };

  exports.getGitVersion = function(callback) {
    var npmOptions;
    npmOptions = {
      userconfig: config.getUserConfigPath(),
      globalconfig: config.getGlobalConfigPath()
    };
    return npm.load(npmOptions, function() {
      var git, outputChunks, spawned, _ref;
      git = (_ref = npm.config.get('git')) != null ? _ref : 'git';
      exports.addGitToEnv(process.env);
      spawned = spawn(git, ['--version']);
      outputChunks = [];
      spawned.stderr.on('data', function(chunk) {
        return outputChunks.push(chunk);
      });
      spawned.stdout.on('data', function(chunk) {
        return outputChunks.push(chunk);
      });
      spawned.on('error', function() {});
      return spawned.on('close', function(code) {
        var gitName, version, versionName, _ref1;
        if (code === 0) {
          _ref1 = Buffer.concat(outputChunks).toString().split(' '), gitName = _ref1[0], versionName = _ref1[1], version = _ref1[2];
          version = version != null ? version.trim() : void 0;
        }
        return callback(version);
      });
    });
  };

}).call(this);
