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
	'tpl/options',
	// modules below this line do not return useful values
	'moment', // Moment.js plugin
	'tiptip' // jQuery plugin
], function ($, _, Backbone, moment, browser, i18n, utils, options_tpl) {

	var bro,
		original_conf,
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

	function updateNow() {
		$('#update-now-span').text(t('library_update_in_progress'));
		sendMessage('optionsUpdateBugList', function (msg) {
			if (!msg.success) {
				$('#update-now-span').text(t('library_update_failed'));
				return;
			}

			if (!msg.is_new_update) {
				$('#update-now-span').html('<span style="color:green">' + t('library_update_already_updated') + '</span>');
				return;
			}

			updateBugsLastUpdated(msg.bugs_last_updated);

			// rebuild the tracker browser
			$('#app-list-reset-search').click();

			createOrResetBrowser(msg.conf.selected_app_ids, msg.db, msg.tagDb, msg.new_app_ids);

			// On update, overwrite original selected ids to prevent unnecessary
			// save warnings, Any changes they made to app ids are wiped out
			original_conf.selected_app_ids = bro.getSelectedAppIds();

			$('#update-now-span').html('<span style="color:green">' + t('library_update_successful') + '</span>');
		});
	}

	function updateBugsLastUpdated(bugs_last_updated) {
		if (bugs_last_updated) {
			$('#apps-last-updated').text(t('library_updated_on',
				moment(+bugs_last_updated).format('LLL')
			));
		}
	}

	function initTabs() {
		$('.tabs li').click(function (e) {
			var $li = $(this);

			// mark this tab active
			$li.addClass('active');
			// and show its contents
			$(this.getAttribute('data-tab-contents-selector')).show();
			$('#buttons').toggle((this.getAttribute('data-tab-contents-selector') != '#about-options'));

			// deactivate all other tabs and hide their contents (found via the
			// anchor's data-tab-contents-selector attribute)
			$li.siblings('li').each(function () {
				var $this = $(this);
				$this.removeClass('active');
				$($this.attr('data-tab-contents-selector')).hide();
			});

			e.preventDefault();
		});
	}

	function showWhitelistError(error_name) {
		if (error_name) {
			$('#whitelist-error-msg')
				.text(t(error_name))
				.show();
			$('#whitelist-error').slideDown({
				duration: 'fast'
			});

		} else {
			$('#whitelist-error-msg').hide();
			$('#whitelist-error').slideUp({
				duration: 'fast'
			});
		}
	}

	function addSiteToWhitelist() {
		var url = this.value.replace(/^http[s]?:\/\//, ''),
			// from node-validator
			isValidUrlRegex = /^(?!mailto:)(?:(?:https?|ftp):\/\/)?(?:\S+(?::\S*)?@)?(?:(?:(?:[1-9]\d?|1\d\d|2[01]\d|22[0-3])(?:\.(?:1?\d{1,2}|2[0-4]\d|25[0-5])){2}(?:\.(?:[1-9]\d?|1\d\d|2[0-4]\d|25[0-4]))|(?:(?:[a-z\u00a1-\uffff0-9]+-?)*[a-z\u00a1-\uffff0-9]+)(?:\.(?:[a-z\u00a1-\uffff0-9]+-?)*[a-z\u00a1-\uffff0-9]+)*(?:\.(?:[a-z\u00a1-\uffff]{2,})))|localhost)(?::\d{2,5})?(?:\/[^\s]*)?$/i;

		var whitelisted_sites = [];
		$('#whitelist option').each(function () {
			whitelisted_sites.push($(this).text());
		});

		// check for validity
		if (url.length >= 2083 || !isValidUrlRegex.test(url)) {
			showWhitelistError('whitelist_error_invalid_url');
			return false;

		// check for dups
		} else if (whitelisted_sites.indexOf(url) >= 0) {
			showWhitelistError('whitelist_error_duplicate_url');
			return false;
		}

		// add to whitelist
		showWhitelistError();
		var option = document.createElement('option');
		option.appendChild(document.createTextNode(url));
		document.getElementById('whitelist').appendChild(option);
		$('#whitelist').change();

		// TODO sort?
		/*
		ghostery.prefs.whitelist.sort(function(host1, host2) {
			function min(a, b) {
				return (a < b) ? a : b;
			}

			components1 = host1.split(".").reverse();
			components2 = host2.split(".").reverse();

			for (i = 0; i < min(components1.length, components2.length); i++) {
				if (components1[i] < components2[i])
					return -1;
				if (components1[i] > components2[i])
					return 1;
			}

			if (components1.length > components2.length)
				return 1;
			if (components2.length > components1.length)
				return -1;

			return 0;
		});
		*/

		this.value = '';
		this.focus();

		return true;
	}

	function initSiteWhitelist(sites) {
		var i, option, whitelist = document.getElementById('whitelist');
		// populate the list of sites
		for (i = 0; i < sites.length; i++) {
			option = document.createElement('option');
			option.appendChild(document.createTextNode(sites[i]));
			whitelist.appendChild(option);
		}
		// add site button
		document.getElementById('whitelist-add-button').addEventListener('click', function (e) {
			addSiteToWhitelist.call(document.getElementById('whitelist-add-input'));
			e.preventDefault();
		});
		// add site by pressing Enter
		$('#whitelist-add-input').keydown(function (e) {
			if (e.which == 13) { // Enter
				addSiteToWhitelist.call(this);
				e.preventDefault();
			}
		});
		// enable/disable the remove buttons
		$('#whitelist').change(function () {
			document.getElementById('whitelist-remove-button').disabled = !$(this).find(':selected').length;
			document.getElementById('whitelist-remove-all-button').disabled = !$(this).find('option').length;
		}).change();
		// remove button
		document.getElementById('whitelist-remove-button').addEventListener('click', function (e) {
			$('#whitelist option:selected').each(function () {
				this.parentNode.removeChild(this);
			});
			$('#whitelist').change();
			e.preventDefault();
		});
		// remove all button
		document.getElementById('whitelist-remove-all-button').addEventListener('click', function (e) {
			$('#whitelist option').each(function () {
				this.parentNode.removeChild(this);
			});
			$('#whitelist').change();
			e.preventDefault();
		});
	}

	function get() {
		var conf = {};

		if ($('#show-alert')[0].checked) {
			conf.show_alert = true;
			conf.alert_bubble_pos = $('#alert-bubble-pos').val();
			conf.alert_bubble_timeout = +$('#alert-bubble-timeout').val();
		} else {
			conf.show_alert = false;
		}

		// TODO standardize on hyphens for CSS attributes
		conf.expand_sources = $('#expand_sources')[0].checked;
		conf.enable_autoupdate = $('#enable_autoupdate')[0].checked;
		conf.ghostrank = $('#ghostrank')[0].checked;

		conf.selected_app_ids = bro.getSelectedAppIds();

		conf.site_whitelist = [];
		$('#whitelist option').each(function () {
			conf.site_whitelist.push($(this).text());
		});

		conf.show_badge = $('#show-badge')[0].checked;
		conf.show_cmp = $('#show-cmp')[0].checked;

		conf.ignore_first_party = $('#ignore-first-party')[0].checked;

		conf.block_by_default = $('#block-by-default')[0].checked;
		conf.notify_library_updates = $('#notify-library-updates')[0].checked;

		if ($('#click2play')[0].checked) {
			conf.enable_click2play = true;
			conf.enable_click2play_social = $('#click2play-buttons')[0].checked;
		} else {
			conf.enable_click2play = false;
			conf.enable_click2play_social = false;
		}

		conf.language = $('#language').val();

		return conf;
	}

	function save() {
		var conf = get();

		$('#saving-options-notice-overlay').fadeIn({
			duration: 'fast',
			complete: function () {
				$('#saving-options-notice').css('visibility', 'visible');
			}
		});

		window.setTimeout(function () {
			sendMessage('optionsSave', conf);
			window.onbeforeunload = null;
			window.close();
		}, 1500);
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

	function loadOptions(bp) {
		initTabs();

		$('#ghostrank-moreinfo-link').click(function (e) {
			e.preventDefault();
			$('#ghostrank-moreinfo').slideDown(null, function () {
				$('#ghostrank-moreinfo-link').parent().hide();
			});
		});

		$('#update-now-link').click(function (e) {
			updateNow();
			e.preventDefault();
		});
		updateBugsLastUpdated(bp.bugs_last_updated);

		browser.init(bp.conf.language);

		createOrResetBrowser(bp.conf.selected_app_ids, bp.db, bp.tagDb, bp.new_app_ids);

		initSiteWhitelist(bp.conf.site_whitelist);

		$('#alert-bubble-help').tipTip({
			content: '<img src="data/images/help/alert_bubble.png" alt="">',
			maxWidth: '300px'
		});
		$('#browser-panel-help').tipTip({
			content: '<img src="data/images/help/panel.png" alt="">',
			maxWidth: '300px'
		});
		$('#badge-help').tipTip({
			content: '<img src="data/images/help/badge.png" alt="">',
			maxWidth: '300px'
		});
		$('#click2play-help').tipTip({
			content: '<img src="data/images/help/c2p.png" alt="">',
			maxWidth: '600px'
		});
		$('#click2play-buttons-help').tipTip({
			content: '<img src="data/images/help/c2p_social_buttons.png" alt="">',
			maxWidth: '300px'
		});

		$('#show-alert').click(function () {
			$('#alert-bubble-sub-options').slideToggle();
		});

		$('#click2play').click(function () {
			$('#show-c2p-buttons').slideToggle();
		});

		$('#whitelist-error-msg-close').click(function () {
			showWhitelistError();
		});

		// licenses
		$('a.license-link').click(function (e) {
			$('body').animate({
				scrollTop: ($(this).offset().top)
			});

			this.style.display = 'none';

			// show the license text
			$('#' + this.id.replace('-link-', '-text-')).slideDown();

			e.preventDefault();
		});

		if (window.location.hash == '#new_trackers') {
			// scroll to tracker browser
			$("html,body").animate({
				scrollTop: $('#tabs-apps').offset().top
			}, 2000, function () {
				// filter by new trackers
				$('#trackers-app-list-filter-type').val('new').change();
			});

		} else if (window.location.hash == '#about') {
			$('#about-tab').trigger('click');

		} else if (window.location.hash == '#alert-bubble-options') {
			// TODO find better way of doing this (:target is not working reliably)
			$('#advanced-tab').trigger('click');
			$('#alert-bubble-options').css('background-color', '#e29400');
			_.defer(function () {
				$('#alert-bubble-options').addClass('highlight');
			});
		}

		// compare current conf to background page's conf, warn if different
		window.onbeforeunload = function () {
			var conf = get();

			// TODO should be able to compare the entire conf object (no loop)
			for (var option in conf) {
				if (!conf.hasOwnProperty(option)) {
					continue;
				}

				if (option == 'selected_app_ids') {
					if (JSON.stringify(original_conf[option]) !== JSON.stringify(conf[option])) {
						return t('options_unsaved_changes_warning');
					}
				} else if (original_conf[option].toString() !== conf[option].toString()) {
					return t('options_unsaved_changes_warning');
				}
			}
		};

		// Save page load conf for warning when leaving the page without saving
		original_conf = get();

		$('.cancel-button').click(function () {
			window.onbeforeunload = null;
			window.open('', '_self', '');
			window.close();
		}).prop('disabled', false);

		$('.save-button').click(save).prop('disabled', false);

		$("#goto-backup").click(function (e) {
			e.preventDefault();
			sendMessage("openBackup", function () {
				window.close();
			});
		});
	}

	// end function definitions //////////////////////////////////////////////////

	sendMessage('optionsLoaded', function (msg) {
		i18n.init(msg.conf.language);
		moment.lang(msg.conf.language.toLowerCase().replace('_', '-'));

		document.title = t('options_page_title');

		var libraries = [
			{
				name: "jQuery",
				url: "http://jquery.com/",
				license_url: "http://jquery.org/license",
				license_text: ['/*!',
					' * jQuery JavaScript Library v1.7.2',
					' * http://jquery.com/',
					' *',
					' * Copyright 2011, John Resig',
					' * Dual licensed under the MIT or GPL Version 2 licenses.',
					' * http://jquery.org/license',
					' *',
					' * Includes Sizzle.js',
					' * http://sizzlejs.com/',
					' * Copyright 2011, The Dojo Foundation',
					' * Released under the MIT, BSD, and GPL Licenses.',
					' *',
					' * Date: Wed Mar 21 12:46:34 2012 -0700',
					' */'].join('\n')
			}, {
				name: "Apprise",
				url: "http://thrivingkings.com/apprise/",
				license_url: "http://creativecommons.org/licenses/by-sa/2.5/",
				license_text: ['// Apprise 1.5 by Daniel Raftery',
					'// http://thrivingkings.com/apprise',
					'//',
					'// Button text added by Adam Bezulski',
					'//'].join('\n')
			}, {
				name: 'Underscore.js',
				url: 'http://documentcloud.github.com/underscore/',
				license_url: "http://www.opensource.org/licenses/mit-license.php",
				license_text: ['//     Underscore.js 1.4.3',
					'//     http://underscorejs.org',
					'//     (c) 2009-2012 Jeremy Ashkenas, DocumentCloud Inc.',
					'//     Underscore may be freely distributed under the MIT license.'].join('\n')
			}, {
				name: 'Backbone.js',
				url: 'http://documentcloud.github.com/backbone/',
				license_url: "http://www.opensource.org/licenses/mit-license.php",
				license_text: ['//     Backbone.js 0.9.10',
					'',
					'//     (c) 2010-2012 Jeremy Ashkenas, DocumentCloud Inc.',
					'//     Backbone may be freely distributed under the MIT license.',
					'//     For all details and documentation:',
					'//     http://backbonejs.org'].join('\n')
			}, {
				name: "TipTip",
				url: "http://code.drewwilson.com/entry/tiptip-jquery-plugin",
				license_url: "http://www.opensource.org/licenses/mit-license.php",
				license_text: ['/*',
					' * TipTip',
					' * Copyright 2010 Drew Wilson',
					' * www.drewwilson.com',
					' * code.drewwilson.com/entry/tiptip-jquery-plugin',
					' *',
					' * Version 1.3   -   Updated: Mar. 23, 2010',
					' *',
					' * This Plug-In will create a custom tooltip to replace the default',
					' * browser tooltip. It is extremely lightweight and very smart in',
					' * that it detects the edges of the browser window and will make sure',
					' * the tooltip stays within the current window size. As a result the',
					' * tooltip will adjust itself to be displayed above, below, to the left ',
					' * or to the right depending on what is necessary to stay within the',
					' * browser window. It is completely customizable as well via CSS.',
					' *',
					' * This TipTip jQuery plug-in is dual licensed under the MIT and GPL licenses:',
					' *   http://www.opensource.org/licenses/mit-license.php',
					' *   http://www.gnu.org/licenses/gpl.html',
					' */'].join('\n')
			}, {
				name: "RequireJS",
				url: "http://requirejs.org/",
				license_url: "http://www.opensource.org/licenses/mit-license.php",
				license_text: ['/**',
					' * @license RequireJS 2.1.2 Copyright (c) 2010-2012, The Dojo Foundation All Rights Reserved.',
					' * Available via the MIT or new BSD license.',
					' * see: http://github.com/jrburke/requirejs for details',
					' */'].join('\n')
			}, {
				name: "Moment.js",
				url: "http://momentjs.com/",
				license_url: "http://www.opensource.org/licenses/mit-license.php",
				license_text: [
					'//! moment.js',
					'//! version : 2.7.0',
					'//! authors : Tim Wood, Iskren Chernev, Moment.js contributors',
					'//! license : MIT',
					'//! momentjs.com'
				].join('\n')
			// TODO review
			}, {
				name: 'node-validator',
				url: 'https://github.com/chriso/node-validator',
				license_url: "http://www.opensource.org/licenses/mit-license.php",
				license_text: ['// Copyright (c) 2010 Chris O\'Hara <cohara87@gmail.com>'].join('\n')
			}
		];

		$('#content').html(
			options_tpl({
				conf: msg.conf,
				languages: i18n.SUPPORTED_LANGUAGES,
				ghostery_version: msg.VERSION,
				libraries: libraries
			})
		);

		loadOptions(msg);
	});

});
