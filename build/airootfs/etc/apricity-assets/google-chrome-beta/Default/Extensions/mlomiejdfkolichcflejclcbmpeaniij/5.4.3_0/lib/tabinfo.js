/*!
 * Ghostery for Chrome
 * http://www.ghostery.com/
 *
 * Copyright 2014 Ghostery, Inc. All rights reserved.
 * See https://www.ghostery.com/eula for license.
 */

define([
	'lib/utils'
], function (utils) {

	/* {
		tab_id: {
			url: string,
			host: string,
			sids: array of surrogate script IDs,
			DOMLoaded: true after DOMContentLoaded,
			partialScan: true if Ghostery was not there from the very start (main_frame load onwards)
		},
		...
	} */
	var tabInfo = {};

	function clear(tab_id) {
		delete tabInfo[tab_id];
	}

	// TODO review
	function create(tab_id, tab_url) {
		var info = {
			partialScan: true
		};

		if (tab_url) {
			info = {
				url: tab_url,
				host: utils.processUrl(tab_url).host,
				sids: [],
				DOMLoaded: false,
				partialScan: false
			};
		}

		info.needsReload = {changes: {}};

		tabInfo[tab_id] = info;
	}

	function get(tab_id) {
		return tabInfo.hasOwnProperty(tab_id) && tabInfo[tab_id];
	}

	return {
		create: create,
		get: get,
		clear: clear
	};

});
