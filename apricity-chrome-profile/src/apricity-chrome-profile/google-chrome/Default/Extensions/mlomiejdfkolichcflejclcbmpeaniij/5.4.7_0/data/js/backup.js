/*!
 * Ghostery for Chrome
 * http://www.ghostery.com/
 *
 * Copyright 2014 Ghostery, Inc. All rights reserved.
 * See https://www.ghostery.com/eula for license.
 */

require([
	'jquery',
	'underscore',
	'backbone',
	'moment',
	'data/lib/browser',
	'lib/i18n',
	'lib/utils',
	'tpl/backup',
	// modules below this line do not return useful values
	'moment', // Moment.js plugin
	'tiptip' // jQuery plugin
], function ($, _, Backbone, moment, browser, i18n, utils, backup_tpl) {

	var t = i18n.t,
		settings;

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

	function generateBackup(settings) {
		var hash = utils.hashCode(JSON.stringify(settings)),
			backup = JSON.stringify({hash: hash, settings: settings}),
			textFileAsBlob = new Blob([backup], {type: 'text/plain'}),
			d = new Date(),
			fileNameToSaveAs = "Ghostery-Backup-" + (d.getMonth() + 1) + "-" + d.getDate() + "-" + d.getFullYear() + ".ghost";

		var downloadLink = document.createElement("a");
		downloadLink.download = fileNameToSaveAs;
		downloadLink.innerHTML = "Download File";
		downloadLink.href = window.webkitURL.createObjectURL(textFileAsBlob);

		downloadLink.click();
	}

	function checkBackup() {
		var backup;

		var fileToLoad = document.getElementById("restore-file").files[0];

		var fileReader = new FileReader();
		fileReader.onload = function (fileLoadedEvent) {
			try {
				backup = JSON.parse(fileLoadedEvent.target.result);

				if (backup.hash !== utils.hashCode(JSON.stringify(backup.settings))) {
					throw "Invalid hash";
				}

				settings = backup.settings;

				$("#restore-error").hide();
				$("#restore-button").show().prop("disabled", false);

			} catch (err) {
				$("#restore-error").show();
				$("#restore-button").hide().prop("disabled", true);
				return;
			}
		};
		fileReader.readAsText(fileToLoad, "UTF-8");
	}

	function restoreBackup() {
		$('#saving-options-notice-overlay').fadeIn({
			duration: 'fast',
			complete: function () {
				$('#saving-options-notice').css('visibility', 'visible');
			}
		});

		settings.conf.alert_bubble_timeout = +settings.conf.alert_bubble_timeout;

		window.setTimeout(function () {
			sendMessage('restoreBackup', {prefs: settings.prefs, conf: settings.conf});
			window.close();
		}, 1500);
	}

	sendMessage('backupLoaded', function (msg) {
		i18n.init(msg.language);
		moment.lang(msg.language.toLowerCase().replace('_', '-'));

		document.title = t('backup_page_title');

		$('#content').html(backup_tpl({}));
		$('#header-title').text(t('backup_page_title'));

		$("#backup-button").click(function () {
			sendMessage('generateBackup', function (msg) {
				generateBackup(msg);
			});
		}).prop("disabled", false);
		$("#restore-button").click(restoreBackup).prop("disabled", true);
		$("#restore-file").change(checkBackup);
		$("#goto-options").click(function (e) {
			e.preventDefault();
			sendMessage("openOptions", function () {
				window.close();
			});
		});
	});
});
