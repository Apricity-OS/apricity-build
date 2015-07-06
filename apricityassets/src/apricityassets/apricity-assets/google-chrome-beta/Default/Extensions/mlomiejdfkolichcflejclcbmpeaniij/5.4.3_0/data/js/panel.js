/*!
 * Ghostery for Chrome
 * http://www.ghostery.com/
 *
 * Copyright 2014 Ghostery, Inc. All rights reserved.
 * See https://www.ghostery.com/eula for license.
 */

require([
	'jquery',
	'data/lib/panel'
], function ($, Panel) {

	function sendMessage(name, message, callback) {
		message = message || {};
		if (typeof message === "function") {
			callback = message;
		}
		callback = callback || function () {};
	
		return (chrome.runtime && chrome.runtime.sendMessage || chrome.extension.sendMessage)({
			name: name,
			message: message
		}, callback);
	}

	sendMessage('panelLoaded', function (message) {
		$('#content').html(Panel.render().el);

		// TODO awkward ... need to initialize language before anything else
		Panel.model.set('language', message.language);
		Panel.model.set(message);
		// TODO find a better place for this, needs to be run after everything is set
		// When inside the change:needsReload listener, pauseBlocking/whitelisted not set yet
		Panel.initializeStartingStates();
	});
});
