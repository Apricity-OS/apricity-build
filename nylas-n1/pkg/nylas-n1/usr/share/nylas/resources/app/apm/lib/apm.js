(function() {
  var child_process, fs, npm, path, semver;

  child_process = require('child_process');

  fs = require('./fs');

  path = require('path');

  npm = require('npm');

  semver = require('npm/node_modules/semver');

  module.exports = {
    getHomeDirectory: function() {
      if (process.platform === 'win32') {
        return process.env.USERPROFILE;
      } else {
        return process.env.HOME;
      }
    },
    getAtomDirectory: function() {
      var _ref;
      return (_ref = process.env.ATOM_HOME) != null ? _ref : path.join(this.getHomeDirectory(), '.atom');
    },
    getCacheDirectory: function() {
      return path.join(this.getAtomDirectory(), '.apm');
    },
    getResourcePath: function(callback) {
      var apmFolder, appFolder, appLocation, asarPath;
      if (process.env.ATOM_RESOURCE_PATH) {
        return process.nextTick(function() {
          return callback(process.env.ATOM_RESOURCE_PATH);
        });
      }
      apmFolder = path.resolve(__dirname, '..');
      appFolder = path.dirname(apmFolder);
      if (path.basename(apmFolder) === 'apm' && path.basename(appFolder) === 'app') {
        asarPath = "" + appFolder + ".asar";
        if (fs.existsSync(asarPath)) {
          return process.nextTick(function() {
            return callback(asarPath);
          });
        }
      }
      apmFolder = path.resolve(__dirname, '..', '..', '..');
      appFolder = path.dirname(apmFolder);
      if (path.basename(apmFolder) === 'apm' && path.basename(appFolder) === 'app') {
        asarPath = "" + appFolder + ".asar";
        if (fs.existsSync(asarPath)) {
          return process.nextTick(function() {
            return callback(asarPath);
          });
        }
      }
      switch (process.platform) {
        case 'darwin':
          return child_process.exec('mdfind "kMDItemCFBundleIdentifier == \'com.github.atom\'"', function(error, stdout, stderr) {
            var appLocation;
            if (stdout == null) {
              stdout = '';
            }
            if (!error) {
              appLocation = stdout.split('\n')[0];
            }
            if (!appLocation) {
              appLocation = '/Applications/Atom.app';
            }
            return callback("" + appLocation + "/Contents/Resources/app.asar");
          });
        case 'linux':
          appLocation = '/usr/local/share/atom/resources/app.asar';
          if (!fs.existsSync(appLocation)) {
            appLocation = '/usr/share/atom/resources/app.asar';
          }
          return process.nextTick(function() {
            return callback(appLocation);
          });
        case 'win32':
          return process.nextTick(function() {
            var programFilesPath;
            programFilesPath = path.join(process.env.ProgramFiles, 'Atom', 'resources', 'app.asar');
            return callback(programFilesPath);
          });
      }
    },
    getReposDirectory: function() {
      var _ref;
      return (_ref = process.env.ATOM_REPOS_HOME) != null ? _ref : path.join(this.getHomeDirectory(), 'github');
    },
    getNodeUrl: function() {
      var _ref;
      return (_ref = process.env.ATOM_NODE_URL) != null ? _ref : 'https://atom.io/download/atom-shell';
    },
    getAtomPackagesUrl: function() {
      var _ref;
      return (_ref = process.env.ATOM_PACKAGES_URL) != null ? _ref : "" + (this.getAtomApiUrl()) + "/packages";
    },
    getAtomApiUrl: function() {
      var _ref;
      return (_ref = process.env.ATOM_API_URL) != null ? _ref : 'https://atom.io/api';
    },
    getNodeVersion: function() {
      var _ref;
      return (_ref = process.env.ATOM_NODE_VERSION) != null ? _ref : '0.22.0';
    },
    getNodeArch: function() {
      switch (process.platform) {
        case 'darwin':
          return 'x64';
        case 'win32':
          return 'ia32';
        default:
          return process.arch;
      }
    },
    getUserConfigPath: function() {
      return path.resolve(this.getAtomDirectory(), '.apmrc');
    },
    getGlobalConfigPath: function() {
      return path.resolve(this.getAtomDirectory(), '.apm', '.apmrc');
    },
    isWin32: function() {
      return process.platform === 'win32';
    },
    isWindows64Bit: function() {
      return fs.existsSync("C:\\Windows\\SysWow64\\Notepad.exe");
    },
    x86ProgramFilesDirectory: function() {
      return process.env["ProgramFiles(x86)"] || process.env["ProgramFiles"];
    },
    getInstalledVisualStudioFlag: function() {
      var vs2010Path, vs2012Path, vs2013Path;
      if (!this.isWin32()) {
        return null;
      }
      if (process.env.GYP_MSVS_VERSION) {
        return process.env.GYP_MSVS_VERSION;
      }
      vs2013Path = path.join(this.x86ProgramFilesDirectory(), "Microsoft Visual Studio 12.0", "Common7", "IDE");
      if (fs.existsSync(vs2013Path)) {
        return '2013';
      }
      vs2012Path = path.join(this.x86ProgramFilesDirectory(), "Microsoft Visual Studio 11.0", "Common7", "IDE");
      if (fs.existsSync(vs2012Path)) {
        return '2012';
      }
      vs2010Path = path.join(this.x86ProgramFilesDirectory(), "Microsoft Visual Studio 10.0", "Common7", "IDE");
      if (fs.existsSync(vs2010Path)) {
        return '2010';
      }
    },
    loadNpm: function(callback) {
      var npmOptions;
      npmOptions = {
        userconfig: this.getUserConfigPath(),
        globalconfig: this.getGlobalConfigPath()
      };
      return npm.load(npmOptions, function() {
        return callback(null, npm);
      });
    },
    getSetting: function(key, callback) {
      return this.loadNpm(function() {
        return callback(npm.config.get(key));
      });
    },
    setupApmRcFile: function() {
      try {
        return fs.writeFileSync(this.getGlobalConfigPath(), "; This file is auto-generated and should not be edited since any\n; modifications will be lost the next time any apm command is run.\n;\n; You should instead edit your .apmrc config located in ~/.atom/.apmrc\ncache = " + (this.getCacheDirectory()) + "\n");
      } catch (_error) {}
    }
  };

}).call(this);
