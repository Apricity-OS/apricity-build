/*!
 * Ghostery for Chrome
 * http://www.ghostery.com/
 *
 * Copyright 2014 Ghostery, Inc. All rights reserved.
 * See https://www.ghostery.com/eula for license.
 */

// TODO move stuff that's not opera/chrome/safari specific to lib/walkthrough.js

require([
	'jquery',
	'underscore',
	'backbone',
	'apprise',
	'data/lib/browser',
	'lib/i18n',
	'lib/utils',
	'tpl/walkthrough'
], function ($, _, Backbone, apprise, browser, i18n, utils, walkthrough_tpl) {

	var bro,
		t = i18n.t;

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

	function saveWalkthrough() {
		var conf = {};

		conf.show_alert = $('#show-alert')[0].checked;
		conf.ghostrank = $('#ghostrank')[0].checked;
		conf.selected_app_ids = bro.getSelectedAppIds();
		conf.block_by_default = $('#block-by-default')[0].checked;

		sendMessage('walkthroughSave', conf);
	}

	function initNavigation() {
		var slides = $('#slider').children(),
			current = 0,
			headerTitles = [
				t('walkthrough_intro_header'),
				'Ghostrank™',
				t('walkthrough_notification_header'),
				t('walkthrough_blocking_header'),
				t('walkthrough_finished1')
			];

		$('#header-title').text(headerTitles[current]);

		function onNavigate() {
			$('#walkthrough-progress').children()
				.removeClass('active')
				.eq(current).addClass('active');

			$('#arrow-prev').toggle(current > 0);
			$('#arrow-next, #skip-button').toggle(current < slides.length - 1);

			saveWalkthrough();

			if (current + 1 == slides.length) {
				sendMessage('walkthroughFinished');
			}
		}

		function next() {
			if (current + 1 >= slides.length) {
				return;
			}

			$(slides[current]).hide();
			$(slides[current + 1]).show();
			$('#header-title').text(headerTitles[current + 1]);

			current++;

			onNavigate();
		}

		function prev() {
			if (!current) {
				return;
			}

			$(slides[current]).hide();
			$(slides[current - 1]).show();
			$('#header-title').text(headerTitles[current - 1]);

			current--;

			onNavigate();
		}

		// clickable arrows
		$('#arrow-prev').click(function (e) {
			prev();
			e.preventDefault();
		}).hide();

		$('#arrow-next').click(function (e) {
			next();
			e.preventDefault();
		});

		// left/right keyboard controls
		$(window).keyup(function (e) {
			// don't navigate when using the tracker browser's name filter input
			if (e.target == $('#app-list-filter-name')[0]) {
				return;
			}
			if (e.keyCode == 37) {
				prev();
			} else if (e.keyCode == 39) {
				next();
			}
		});
	}

	function createOrResetBrowser(selected_app_ids, bugDb, tagDb, new_app_ids) {
		if (!bro) {
			bro = new browser.AppBrowser({
				el: $('#trackers-app-browser'),
				categories: new browser.Categories(
					browser.getCategories(bugDb, selected_app_ids, tagDb)
				),
				new_app_ids: new_app_ids
			});
		} else {
			bro.categories.reset(
				browser.getCategories(bugDb, selected_app_ids, tagDb)
				);
			bro.initializeTags();
			bro.new_app_ids = new_app_ids;
		}
	}

	function loadWalkthrough(bp) {
		i18n.init(bp.conf.language);

		document.title = t('walkthrough_page_title');

		$('#content').html(
			walkthrough_tpl({
				conf: bp.conf
			})
		);

		browser.init(bp.conf.language);

		createOrResetBrowser(bp.conf.selected_app_ids, bp.db, bp.tagDb, bp.new_app_ids);

		$('#version-text').text(utils.VERSION);

		$('#skip-button').click(function (e) {
			e.preventDefault();

			apprise(t('walkthrough_skip_confirmation'), {
				confirm: true,
				textOk: t('button_ok'),
				textCancel: t('button_cancel')
			}, function (r) {
				if (r) {
					sendMessage('walkthroughAborted');
				}
			});
		});

		$('#ghostrank-moreinfo-link').click(function (e) {
			e.preventDefault();
			$('#ghostrank-moreinfo').slideDown(null, function () {
				$('#ghostrank-moreinfo-link').parent().hide();
			});
		});

		window.onbeforeunload = function () {
			sendMessage('record_install');
		};

		initNavigation();
	}

	// end function definitions //////////////////////////////////////////////////

	sendMessage('optionsLoaded', function (message) {
		// TODO should this be inside ondomready or whatever?
		// TODO review order of things (to speed up page rendering)
		loadWalkthrough(message);
	});

});
