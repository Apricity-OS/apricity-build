/*!
 * Ghostery for Chrome
 * http://www.ghostery.com/
 *
 * Copyright 2014 Ghostery, Inc. All rights reserved.
 * See https://www.ghostery.com/eula for license.
 */

define([
	'require',
	'underscore',
	'lib/utils'
], function (require, _, utils) {

	var db = {
		pattern_ids: {},
		app_ids: {},
		site_surrogates: {}
	};

	function buildDb(surrogate, property, db_name) {
		// take arrays of app IDs/pattern IDs/site domains and index them by
		// their values for easy lookup
		surrogate[property].forEach(function (val) {
			if (!db[db_name].hasOwnProperty(val)) {
				db[db_name][val] = [];
			}
			db[db_name][val].push(surrogate);
		});
	}

	function init() {
		var data = JSON.parse(utils.syncGet('data/databases/surrogates.json'));

		data.mappings.forEach(function (s) {
			s.code = data.surrogates[s.sid];

			// convert single values to arrays first
			['pattern_id', 'app_id', 'sites', 'match'].forEach(function (prop) {
				if (s.hasOwnProperty(prop) && !_.isArray(s[prop])) {
					s[prop] = [s[prop]];
				}
			});

			// initialize regexes
			if (s.hasOwnProperty('match')) {
				s.match = _.map(s.match, function (match) {
					return new RegExp(match, '');
				});
			}

			if (s.hasOwnProperty('pattern_id') || s.hasOwnProperty('app_id')) {
				// tracker-level surrogate
				if (s.hasOwnProperty("pattern_id")) {
					buildDb(s, "pattern_id", "pattern_ids");
				} else if (s.hasOwnProperty("app_id")) {
					buildDb(s, "app_id", "app_ids");
				}
			} else {
				// we have a "sites" property, but not pattern_id/app_id:
				// it's a site surrogate
				if (s.hasOwnProperty("sites")) {
					buildDb(s, "sites", "site_surrogates");
				}
			}
		});
	}

	function getForSite(host_name) {
		var surrogates = [];

		// note: does not support *.example.com (exact matches only)
		if (db.site_surrogates.hasOwnProperty(host_name)) {
			surrogates = db.site_surrogates[host_name];
		}

		return surrogates;
	}

	function getForTracker(script_src, app_id, pattern_id, host_name) {
		var candidates = [];

		if (db.app_ids.hasOwnProperty(app_id)) {
			candidates = candidates.concat(db.app_ids[app_id]);
		}

		if (db.pattern_ids.hasOwnProperty(pattern_id)) {
			candidates = candidates.concat(db.pattern_ids[pattern_id]);
		}

		return _.filter(candidates, function (surrogate) {
			// note: does not support *.example.com (exact matches only)
			if (surrogate.hasOwnProperty("sites")) { // array of site hosts
				if (surrogate.sites.indexOf(host_name) == -1) {
					return false;
				}
			}

			if (surrogate.hasOwnProperty("match")) {
				if (!_.any(surrogate.match, function (match) {
					return script_src.match(match);
				})) {
					return false;
				}
			}

			return true;
		});
	}

	init();

	return {
		getForTracker: getForTracker,
		getForSite: getForSite
	};

});
