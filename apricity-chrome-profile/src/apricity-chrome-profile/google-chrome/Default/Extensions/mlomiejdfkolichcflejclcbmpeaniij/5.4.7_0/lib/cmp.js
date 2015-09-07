/*!
 * Ghostery for Opera
 * http://www.ghostery.com/
 *
 * Copyright 2014 EVIDON, Inc. All rights reserved.
 * See https://www.ghostery.com/eula for license.
 */

define([
	'jquery',
	'lib/conf',
	'lib/utils',
], function ($, conf, utils) {

	function fetchMktgData(callback) {
		if (!conf.show_cmp) { return; }
		chrome.runtime.getPlatformInfo(function (pi) {
			var URL = 'https://cmp-cdn.ghostery.com/check' +
				'?os=' + encodeURIComponent(pi.os) +
				'&gr=' + encodeURIComponent((conf.ghostrank ? 'opt-in' : 'opt-out')) +
				'&ua=chrome' +
				'&v=' + encodeURIComponent(utils.prefs('cmp_version') || 0);

			$.ajax({
				dataType: 'json',
				url: URL,
				complete: function (xhr, status) {
					var data;

					if (status == 'success' && xhr.status != 204) { // check for success and no content
						try {
							data = JSON.parse(xhr.responseText);
						} catch (e) {
							console.log(e);
						}
					}

					if (data && (!utils.prefs('cmp_version') || data.Version > utils.prefs('cmp_version'))) { // success
						utils.prefs('cmp_version', data.Version);
						// set default dismiss
						data.Campaigns.forEach(function (c) {
							if (c.Dismiss === 0) {
								c.Dismiss = 10;
							}
						});
						utils.prefs('cmp_data', data.Campaigns);
						if (callback) {
							callback();
						}
					} else { // error
						utils.prefs('cmp_data', []);
					}
				}
			});
		});
	}

	return {
		fetchMktgData: fetchMktgData
	};

});
