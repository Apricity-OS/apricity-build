(function() {
  var detailedLog, detailedLogging, fs, path, request;

  fs = require('fs');

  path = require('path');

  request = require('request');

  detailedLogging = false;

  detailedLog = function(msg) {
    if (detailedLogging) {
      return console.log(msg);
    }
  };

  module.exports = function(dir, regexPattern) {
    var callback;
    callback = this.async();
    console.log("Running log ship: " + dir + ", " + regexPattern);
    return fs.readdir(dir, function(err, files) {
      var AWS, AWSModulePath, bucket, file, filepath, finished, logFilter, logs, remaining, stats, uploadTime, _i, _len;
      if (err) {
        log("readdir error: " + err);
      }
      logs = [];
      logFilter = new RegExp(regexPattern);
      for (_i = 0, _len = files.length; _i < _len; _i++) {
        file = files[_i];
        if (logFilter.test(file)) {
          filepath = path.join(dir, file);
          stats = fs.statSync(filepath);
          if (stats["size"] > 0) {
            logs.push(filepath);
          }
        }
      }
      remaining = 0;
      finished = function() {
        remaining -= 1;
        if (remaining === 0) {
          return callback();
        }
      };
      if (logs.length === 0) {
        detailedLog("No logs found to upload.");
        callback();
        return;
      }
      if (__dirname.indexOf('app.asar') !== -1) {
        AWSModulePath = path.join(__dirname, '..', '..', '..', 'app.asar.unpacked', 'node_modules', 'aws-sdk');
      } else {
        AWSModulePath = 'aws-sdk';
      }
      AWS = require(AWSModulePath);
      AWS.config.update({
        accessKeyId: 'AKIAIEGVDSVLK3Z7UVFA',
        secretAccessKey: '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'
      });
      bucket = new AWS.S3({
        params: {
          Bucket: 'edgehill-client-logs'
        }
      });
      uploadTime = Date.now();
      return logs.forEach(function(log) {
        var key, params, stream;
        stream = fs.createReadStream(log, {
          flags: 'r'
        });
        key = "" + uploadTime + "-" + (path.basename(log));
        params = {
          Key: key,
          Body: stream
        };
        remaining += 1;
        return bucket.upload(params, function(err, data) {
          if (err) {
            detailedLog("Error uploading " + key + ": " + (err.toString()));
          } else {
            detailedLog("Successfully uploaded " + key);
          }
          fs.truncate(log);
          return finished();
        });
      });
    });
  };

}).call(this);
