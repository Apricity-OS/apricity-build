/*!
 * Ghostery for Chrome
 * http://www.ghostery.com/
 *
 * Copyright 2014 Ghostery, Inc. All rights reserved.
 * See https://www.ghostery.com/eula for license.
 */

define([
	'lib/conf',
	'lib/utils'
], function (conf, utils) {

	function sendReq(kind) {
		chrome.runtime.getPlatformInfo(function (sysinfo) {
			// Since this is async, we need to recheck if already sent.
			if (kind === 'install' && utils.prefs('install_recorded')) { return; }

			var xhr,
				metrics_url = 'https://d.ghostery.com/' + kind +
				'?gr=' + (conf.ghostrank ? '1' : '0') +
				'&v=' + encodeURIComponent(utils.VERSION) +
				'&os=' + encodeURIComponent(sysinfo.os) +
				'&ua=ch';
			utils.log('XHR to ' + metrics_url);

			xhr = new XMLHttpRequest();
			xhr.open("GET", metrics_url, true);
			xhr.send();

			// setting this every time doesn't hurt, plus it sets it on upgrade
			// when upgrading from versions without the install ping
			utils.prefs('install_recorded', true);
		});
	}

	function recordInstall() {
		// not necessary but prevents unnecessary calls to getPlatformInfo() in sendReq
		if (utils.prefs('install_recorded')) { return; }

		sendReq('install');
	}

	function recordUpgrade() {
		sendReq('upgrade');
	}

	return {
		recordInstall: recordInstall,
		recordUpgrade: recordUpgrade
	};

});
