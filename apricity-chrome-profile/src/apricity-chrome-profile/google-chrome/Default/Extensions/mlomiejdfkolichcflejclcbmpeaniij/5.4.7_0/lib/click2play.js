/*!
 * Ghostery for Chrome
 * http://www.ghostery.com/
 *
 * Copyright 2014 Ghostery, Inc. All rights reserved.
 * See https://www.ghostery.com/eula for license.
 */

define([
	'underscore',
	'lib/utils',
	'lib/updatable'
], function (_, utils, UpdatableMixin) {

	var log = utils.log;

	function Click2playDb() {
		this.type = 'click2play';

		var allowOnceList = {};

		function buildDb(entries, version) {
			var	apps = {},
				allow;

			entries.forEach(function (entry) {
				if (!apps.hasOwnProperty(entry.aid)) {
					apps[entry.aid] = [];
				}

				allow = [
					entry.aid
				];
				if (entry.alsoAllow) {
					allow = allow.concat(entry.alsoAllow);
				}

				apps[entry.aid].push({
					aid: entry.aid,
					allow: allow,
					frameColor: (entry.frameBackground ? entry.frameBackground : ''),
					text: (entry.text ? entry.text : ''),
					button: (entry.button ? entry.button : ''),
					attach: (entry.attach ? entry.attach : false),
					ele: (entry.selector ? entry.selector : ''),
					type: (entry.type ? entry.type : '')
				});
			});

			return {
				apps: apps,
				version: version
			};
		}

		this.processList = function (data) {
			var db;

			log('processing c2p ...');

			try {
				db = buildDb(data.click2play, data.click2playVersion);
			} catch (e) {}

			if (!db) {
				return false;
			}

			log('processed');

			this.db = db;
			utils.prefs('click2play', data);

			return true;
		};

		// TODO memory leak when you close tabs before reset() can run?
		this.reset = function (tab_id) {
			if (!allowOnceList.hasOwnProperty(tab_id)) {
				return;
			}

			var list = allowOnceList[tab_id];

			for (var aid in list) {
				if (!list.hasOwnProperty(aid)) {
					continue;
				}
				list[aid]--;
				if (list[aid] <= 0) {
					delete list[aid];
				}
			}

			if (!_.size(list)) {
				delete allowOnceList[tab_id];
			}
		};

		this.allowedOnce = function (tab_id, aid) {
			return allowOnceList.hasOwnProperty(tab_id) &&
				allowOnceList[tab_id].hasOwnProperty(aid);
		};

		this.allowOnce = function (app_ids, tab_id) {
			if (!allowOnceList.hasOwnProperty(tab_id)) {
				allowOnceList[tab_id] = {};
			}

			// the page will be reloaded, but we want to keep allowing
			// everything else that's been "allowed once"
			for (var aid in allowOnceList[tab_id]) {
				if (allowOnceList[tab_id].hasOwnProperty(aid)) {
					allowOnceList[tab_id][aid]++;
				}
			}

			// for all allow and alsoAllow app IDs, initialize allowOnce
			// counters to 2 to allow for a page reload
			app_ids.forEach(function (app_id) {
				allowOnceList[tab_id][app_id] = 2;
			});
		};

		_.extend(this, UpdatableMixin);
	}

	return new Click2playDb();

});
