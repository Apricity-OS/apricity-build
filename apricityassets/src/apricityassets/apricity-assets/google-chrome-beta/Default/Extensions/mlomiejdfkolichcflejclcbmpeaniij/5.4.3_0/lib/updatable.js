/*!
 * Ghostery for Chrome
 * http://www.ghostery.com/
 *
 * Copyright 2014 Ghostery, Inc. All rights reserved.
 * See https://www.ghostery.com/eula for license.
 */

define([
	'jquery',
	'underscore',
	'lib/utils'
], function ($, _, utils) {

	this.db = {};

	var UpdatableMixin = {

		_localFetcher: function () {
			var memory = utils.prefs(this.type),
				version_property = (this.type == 'bugs' ? 'version' : (this.type + 'Version'));

			// nothing in storage, or it's so old it doesn't have a version
			if (!memory || !memory.hasOwnProperty(version_property)) {
				// return what's on disk
				utils.log('fetching ' + this.type + ' from disk');
				return JSON.parse(utils.syncGet('data/databases/' + this.type + '.json'));
			}

			// on upgrades, see if bugs.json shipped w/ the extension is more recent
			if (this.just_upgraded) {
				var disk = JSON.parse(utils.syncGet('data/databases/' + this.type + '.json'));

				if (disk[version_property] > memory[version_property]) {
					utils.log('fetching ' + this.type + ' from disk');
					return disk;
				}
			}

			utils.log('fetching ' + this.type + ' from memory');
			return memory;
		},

		_downloadList: function (callback) {
			var UPDATE_URL = 'https://cdn.ghostery.com/update/' +
				(this.type == 'bugs' ? 'v2/bugs' : this.type);

			$.ajax({
				dataType: 'json',
				url: UPDATE_URL,
				complete: function (xhr, status) {
					var list;

					if (status == 'success') {
						try {
							list = JSON.parse(xhr.responseText);
						} catch (e) {}
					}

					if (list) { // success
						callback(true, list);
					} else { // error
						callback(false);
					}
				}
			});
		},

		// asynchronous
		_remoteFetcher: function (callback) {
			utils.log('fetching ' + this.type + ' from remote');
			this._downloadList(callback);
		},

		// both fetchers return a bugs object to be processed
		// TODO strategy pattern?
		_loadList: function (options) {
			options = options || {};

			// synchronous
			// TODO make async for consistency w/ remote fetching
			if (!options.remote) {
				return this.processList(
					this._localFetcher(),
					!!options.initializing
				);
			}

			if (this.db.version && options.version && options.version == this.db.version) {
				//already up-to-date
				if (options.callback) {
					options.callback({
						'success': true,
						'updated': false
					});
				}
				utils.prefs(this.type + '_last_updated', (new Date()).getTime());

				return;
			}

			// asynchronous
			this._remoteFetcher(_.bind(function (result, list) {
				if (result && list) {
					result = this.processList(list);
				}

				if (result) {
					// note: only when fetching from ghostery.com
					utils.prefs(this.type + '_last_updated', (new Date()).getTime());
				}

				if (options.callback) {
					// TODO notify the user in the nothing changed case
					// TODO if we stop updating bugs_last_updated in the nothing changed case,
					// TODO let's make sure to fix the autoupdate logic
					options.callback({
						'success': result,
						'updated': (result ? true : false)
					});
				}
			}, this));
		},

		update: function (version, callback) {
			var opts = {
				remote: true,
				version: version,
				callback: callback
			};

			if (_.isFunction(version)) {
				opts.callback = version;
				delete opts.version;
			}

			this._loadList(opts);
		},

		init: function (just_upgraded) {
			this.just_upgraded = just_upgraded;
			return this._loadList({
				initializing: true
			});
		}
	};

	return UpdatableMixin;

});
