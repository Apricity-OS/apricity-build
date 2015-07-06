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
	'lib/bugdb',
	'lib/click2play',
	'lib/compatibility',
	'lib/tagdb',
	'lib/conf',
	'lib/dispatcher',
	'lib/metrics',
	'lib/foundbugs',
	'lib/ghostrank',
	'lib/i18n',
	'lib/matcher',
	'lib/surrogatedb',
	'lib/tabinfo',
	'lib/utils',
	'tpl/click2play'
], function ($, _, bugDb, c2pDb, compDb, tagDb, conf, dispatcher, metrics, foundBugs, ghostrank, i18n, matcher, surrogatedb, tabInfo, utils, c2p_tpl) {

	var prefs = utils.prefs,
		log = utils.log,
		upgrade_alert_shown = false,
		// Chrome 20-25 uses chrome.extension.sendMessage, Chrome >= 26 uses chrome.runtime.sendMessage
		// TODO verify on Chrome 23-25 (our minimum version is now 23)
		onMessage = chrome.runtime && chrome.runtime.onMessage || chrome.extension.onMessage;

	// are we running for the first time/upgrading?
	// TODO move into init()?
	var JUST_INSTALLED = !localStorage.previousVersion,
		JUST_UPGRADED = localStorage.previousVersion != utils.VERSION && !JUST_INSTALLED;
	localStorage.previousVersion = utils.VERSION;

	if (JUST_UPGRADED) {
		metrics.recordUpgrade();
	} else if (JUST_INSTALLED) {
		setTimeout(function () {
			metrics.recordInstall();
		}, 300000);
	} else {
		metrics.recordInstall();
	}

	// tracker latencies (unblocked requests belonging to trackers)
	// { request_id: { start_time: string, bug_id: int } }
	// TODO remove window.latencies, (testing for uncleared entries)
	var latencies = window.latencies = {};

	window.perfs = [];

	function clearTabData(tab_id) {
		foundBugs.clear(tab_id);
		tabInfo.clear(tab_id);
	}

	// TODO speed up
	function whitelisted(url) {
		var i, num_sites = conf.site_whitelist.length;
		for (i = 0; i < num_sites; i++) {
			// TODO match from the beginning of the string to avoid false matches (somewhere in the querystring for instance)
			if (url.indexOf(conf.site_whitelist[i]) >= 0) {
				return conf.site_whitelist[i];
			}
		}
		return false;
	}

	function checkLibraryVersion(callback) {
		var VERSION_CHECK_URL = "https://cdn.ghostery.com/update/version";

		// TODO this does not handle no response/404/bad JSON
		$.getJSON(VERSION_CHECK_URL, function (r) {
			bugDb.update(r.bugsVersion, callback);
			c2pDb.update(r.click2playVersion);
			compDb.update(r.compatibilityVersion);
			tagDb.update(r.tagsVersion);
		});
	}

	function autoUpdateBugDb() {
		// check and fetch (if needed) a new tracker library every 12 hours
		if (conf.enable_autoupdate) {
			if (!prefs('bugs_last_updated') ||
				(new Date()) > (new Date(+prefs('bugs_last_updated') + (1000 * 60 * 60 * 12)))) {
				checkLibraryVersion();
			}
		}
	}

	function saveOptions(message) {
		$.each(conf.toJSON(), function (setting) {
			if (typeof message[setting] != 'undefined') {
				conf[setting] = message[setting];
			}
		});
	}

	function openTab(url, isExtPage) {
		if (isExtPage) {
			url = getURL(url);
		}
		utils.getActiveTab(function (tab) {
			chrome.tabs.create({
				url: url,
				openerTabId: tab.id,
				index: tab.index + 1
			});
		});
	}

	function getURL(url) {
		return chrome.extension.getURL(url);
	}

	function sendMessage(tab_id, name, message, callback) {
		chrome.tabs.sendMessage(tab_id, {
			name: name,
			message: message
		}, callback);
	}

	function injectedScriptMessageListener(name, message, tab_id, tab_url, sendResponse) {
		var response = {};

		//if (!tab_id) {
		//	sendResponse({});
		//	return;
		//}

		if (name == 'openWalkthrough') {
			openTab('walkthrough.html', true);

		} else if (name == 'showNewTrackers') {
			openTab('options.html#new_trackers', true);

		} else if (name == 'openOptions') {
			openTab('options.html', true);

		} else if (name == 'openAbout') {
			openTab('options.html#about', true);

		} else if (name == 'openBackup') {
			openTab('backup.html', true);

		} else if (name == 'openTab') {
			openTab(message.url);

		} else if (name == 'optionsLoaded') {
			response =  {
				db: bugDb.db,
				tagDb: tagDb.db,
				bugs_last_updated: prefs('bugs_last_updated'),
				conf: conf.toJSON(),
				new_app_ids: prefs('newAppIds'),
				VERSION: utils.VERSION
			};

		} else if (name == 'showPurpleBoxOptions') {
			openTab('options.html#alert-bubble-options', true);

		} else if (name == 'recordPageInfo') {
			ghostrank.recordPage(message.domain, message.latency, message.spots);

		} else if (name == 'optionsSave' || name == 'walkthroughSave') {
			saveOptions(message);

		} else if (name == 'walkthroughAborted' || name == 'walkthroughFinished') {
			if (JUST_INSTALLED) {
				metrics.recordInstall();
			}
			prefs(name, true);
			if (name == 'walkthroughAborted') {
				chrome.tabs.remove(tab_id, null);
			}

		} else if (name == 'record_install') {
			metrics.recordInstall();

		} else if (name == 'optionsUpdateBugList') {
			checkLibraryVersion(function (result) {
				sendResponse({
					db: bugDb.db,
					tagDb: tagDb.db,
					bugs_last_updated: prefs('bugs_last_updated'),
					conf: conf.toJSON(),
					new_app_ids: prefs('newAppIds'),
					success: result.success,
					is_new_update: result.updated
				});
			});

			// Makes response wait for asynchronous response above
			return true;

		} else if (name == 'pageInjected') {
			// TODO c2p
			// the rest is for top-level documents only
			//if (message.from_frame) {
			//	return;
			//}

		} else if (name == 'panelLoaded') {
			// TODO verify that these findings panel messages are always for the active tab
			// TODO verify that tabInfo will always have the stuff we need for the active tab ID
			utils.getActiveTab(function (tab) {
				var data = {
					trackers: foundBugs.getApps(tab.id, false, tab.url),
					conf: conf.toJSON(),
					page: {
						url: tab.url,
						host: tabInfo.get(tab.id).host
					},
					whitelisted: whitelisted(tab.url),
					pauseBlocking: conf.paused_blocking,
					needsReload: (tabInfo.get(tab.id) ? tabInfo.get(tab.id).needsReload : { changes: {} }),
					showTutorial: !utils.prefs('panelTutorialShown'),
					// TODO "validProtocol" doesn't really apply to chrome, does it?
					validProtocol: (tab.url.indexOf('http') === 0 && foundBugs.getBugs(tab.id) !== false),
					language: conf.language
				};

				sendResponse(data);
			});
			// Wait for async response above
			return true;

		} else if (name == 'panelSelectedAppsUpdate') {
			if (message.app_selected) {
				conf.selected_app_ids[message.app_id] = 1;
			} else {
				delete conf.selected_app_ids[message.app_id];
			}

		} else if (name == 'panelSiteWhitelistToggle') {
			utils.getActiveTab(function (tab) {
				var whitelisted_url = whitelisted(tabInfo.get(tab.id).url),
					hostname = tabInfo.get(tab.id).host;

				if (whitelisted_url) {
					conf.site_whitelist.splice(conf.site_whitelist.indexOf(whitelisted_url), 1);
				} else if (hostname) {
					conf.site_whitelist.push(hostname);
				}
			});

		} else if (name == 'panelSiteSpecificUnblockUpdate') {
			utils.getActiveTab(function (tab) {
				var app_id = +message.app_id,
					host = tabInfo.get(tab.id).host;

				if (message.siteSpecificUnblocked) {
					conf.addSiteSpecificUnblock(host, app_id);
				} else {
					conf.removeSiteSpecificUnblock(host, app_id);
				}
			});

		} else if (name == 'processC2P') {
			// TODO do we need this check here?
			if (!tab_id) {
				sendResponse({});
				return;
			}

			if (message.action == 'always') {
				message.app_ids.forEach(function (aid) {
					if (conf.selected_app_ids.hasOwnProperty(aid)) {
						delete conf.selected_app_ids[aid];
					}
				});
				sendMessage(tab_id, 'reload');

			} else if (message.action == 'once') {
				c2pDb.allowOnce(message.app_ids, tab_id);
				sendMessage(tab_id, 'reload');
			}

		} else if (name == 'panelPauseToggle') {
			conf.paused_blocking = !conf.paused_blocking;

		} else if (name == 'panelShowTutorialSeen') {
			prefs('panelTutorialShown', true);

		} else if (name == 'needsReload') {
			utils.getActiveTab(function (tab) {
				// TODO do we need the check?
				if (tabInfo.get(tab.id)) {
					tabInfo.get(tab.id).needsReload = message.needsReload;
				}
			});

		} else if (name == 'reloadTab') {
			utils.getActiveTab(function (tab) {
				sendMessage(tab.id, 'reload');
			});
		} else if (name == 'backupLoaded') {
			response = {
				language: conf.language,
			};

		} else if (name == 'generateBackup') {
			response = {
				conf: conf.toJSON(),
				prefs: {
					panelTutorialShown: prefs('panelTutorialShown'),
					walkthroughFinished: prefs('walkthroughFinished'),
					walkthroughAborted: prefs('walkthroughAborted')
				}
			};

		} else if (name == 'restoreBackup') {
			saveOptions(message.conf);

			for (var p in message.prefs) {
				prefs(p, message.prefs[p]);
			}
		} else if (name == 'appsPageLoaded') {
			utils.getActiveTab(function (tab) {
				sendMessage(tab.id, 'appsPageData', { blocked: conf.selected_app_ids[message.id] == 1 });
			});
		}



		sendResponse(response);
	}

	function initDispatcher() {
		dispatcher.on('conf.save.selected_app_ids', function (v) {
			var num_selected = _.size(v),
				db = bugDb.db;
			db.noneSelected = (num_selected === 0);
			// can't simply compare num_selected and _.size(db.apps) since apps get removed sometimes
			db.allSelected = (!!num_selected && _.every(db.apps, function (app, app_id) {
				return v.hasOwnProperty(app_id);
			}));
		});
		dispatcher.on('conf.save.site_whitelist', function () {
			// TODO why aren't these in Safari?
			// TODO debounce with below
			updateButton();
			utils.flushChromeMemoryCache();
		});
		dispatcher.on('conf.save.paused_blocking', function () {
			// TODO debounce with above
			updateButton();
			utils.flushChromeMemoryCache();
		});
		dispatcher.on('conf.save.language', function (v) {
			i18n.init(v);
		});

	}

	function logLatency(deets, success) {
		var request_id = deets.requestId,
			tab_id = deets.tabId,
			bug_id,
			start_time,
			page_url,
			incognito;

		if (!conf.ghostrank) {
			return;
		}

		if (!latencies.hasOwnProperty(request_id)) {
			return;
		}

		// If the latencies object for this request id is empty then this is
		// not a tracker. Safe to delete object and return.
		if (_.isEmpty(latencies[request_id])) {
			delete latencies[request_id];
			return;
		}

		// TRACKER1 --> NON-TRACKER --> TRACKER2
		// TRACKER2's onBeforeRequest sync callback could maybe fire before
		// NON-TRACKER's onBeforeRedirect async callback
		if (!latencies[request_id].hasOwnProperty(deets.url)) {
			return;
		}

		start_time = latencies[request_id][deets.url].start_time;
		bug_id = latencies[request_id][deets.url].bug_id;
		// These could be undefined if tabInfo wasn't ready before
		page_url = latencies[request_id][deets.url].page_url;
		incognito = latencies[request_id][deets.url].incognito;

		delete latencies[request_id][deets.url];
		if (_.isEmpty(latencies[request_id])) {
			delete latencies[request_id];
		}

		var response_code = deets.statusCode || -1,
			latency = Math.round(deets.timeStamp - start_time),
			from_cache = deets.fromCache ? 1 : 0,
			user_error = success ? 0 : 1,
			blocked =
				deets.error === "net::ERR_BLOCKED_BY_CLIENT" ||
				(deets.redirectUrl && deets.redirectUrl.indexOf("http") !== 0);

		// special case for blocked by another extension, treat as blocked normally
		// if they want, they can look at 'bl' to see if blocked by Ghostery
		if (blocked) {
			latency = -1;
			user_error = -1;
			from_cache = -1;
		}

		// On some errors the tab doesn't exist anymore
		// TODO do we need this check? See below
		if (page_url !== undefined && incognito !== undefined) {
			if (!incognito) {
				ghostrank.record(
						page_url,
						deets.url,
						bug_id,
						false,
						latency,
						response_code,
						user_error,
						from_cache
						);
			}
		} else {
			// This only happens if tabInfo wasn't ready for this tab
			// when this tracker was added to latencies
			// TODO Does it actually ever hit this? How often?
			chrome.tabs.get(tab_id, function (tab) {
				if (tab && !tab.incognito) {
					ghostrank.record(
						tab.url,
						deets.url,
						bug_id,
						false,
						latency,
						response_code,
						user_error,
						from_cache
						);
				}
			});
		}
	}

	function onDOMContentLoaded(tab_id) {
		// show alert bubble only after DOM has loaded
		if (tabInfo.get(tab_id)) {
			tabInfo.get(tab_id).DOMLoaded = true;
		}
		if (conf.show_alert) {
			if (!JUST_UPGRADED || upgrade_alert_shown) {
				showAlert(tab_id);
			}
		}

		// show upgrade notifications
		utils.getActiveTab(function (tab) {
			if (tab.id != tab_id || tab.incognito) {
				return;
			}

			var alert_messages = [
				'dismiss',
				'notification_reminder1',
				'notification_reminder2',
				'notification_reminder_link',
				'notification_update',
				'notification_update_link',
				'notification_upgrade'
			];

			if (JUST_UPGRADED && !upgrade_alert_shown) {
				var name = 'showUpgradeAlert';

				// Ghostrank is off and we've already dismissed or finished the walkthrough
				if (!conf.ghostrank && (prefs('walkthroughAborted') || prefs('walkthroughFinished'))) {
					name = 'showWalkthroughAlert';
				}

				sendMessage(tab_id, name, {
						translations: _.object(_.map(alert_messages, function (key) { return [key, i18n.t(key)]; }))
					},
					function () {
						// not all tabs will have content scripts loaded, so better wait for confirmation first
						// TODO no longer necessary?
						upgrade_alert_shown = true;
					});

			} else if (bugDb.db.JUST_UPDATED_WITH_NEW_TRACKERS) {
				if (conf.notify_library_updates) {
					sendMessage(tab_id, 'showUpdateAlert', {
							translations: _.object(_.map(alert_messages, function (key) { return [key, i18n.t(key)]; }))
						},
						function () {
							bugDb.db.JUST_UPDATED_WITH_NEW_TRACKERS = false;
						});

				} else {
					bugDb.db.JUST_UPDATED_WITH_NEW_TRACKERS = false;
				}
			}
		});

		// perform page-level Ghostrank, but only if the page had some trackers on it
		if (conf.ghostrank && foundBugs.getAppsCount(tab_id) > 0) {
			chrome.tabs.get(tab_id, function (tab) {
				if (tab && !tab.incognito && ghostrank.isValidUrl(utils.processUrl(tab.url))) {
					chrome.tabs.executeScript(tab.id, {
						file: 'data/includes/page_info.js',
						runAt: 'document_idle'
					});
				}
			});
		}
	}

	// TODO THIS HAS TO BE SUPER FAST
	// TODO speed this up by making it asynchronous when blocking is disabled?
	// TODO also speed it up for blocking-whitelisted pages (by delaying isBug scanning)?
	function onBeforeRequest(deets) {
		var tab_id = deets.tabId,
			request_id = deets.requestId;

		if (tab_id <= 0) {
			return;
		}

		// TODO GHOST-451: this is likely to not be ready in time
		// for pages being reopened on browser startup
		if (deets.type == 'main_frame') {
			// TODO GHOST-833 handle pushState navigation properly
			log("❤ ❤ ❤ Tab %s navigating to %s ❤ ❤ ❤", tab_id, deets.url);

			// TODO in safari:
			/*
			// handle reloads
			if (current_url == nav_url) {
				clearTabData(tab_id);
			}
			*/
			clearTabData(tab_id);

			tabInfo.create(tab_id, deets.url);

			// Save whether incognito for ghostrank
			chrome.tabs.get(tab_id, function (tab) {
				// preloading tabs have no tab object, get active tab's incognito state
				if (tab) {
					tabInfo.get(tab_id).incognito = tab.incognito;
				} else {
					utils.getActiveTab(function (tab) {
						tabInfo.get(tab_id).incognito = tab.incognito;
					});
				}
			});

			ghostrank.onNavigate(deets.url);

			// TODO crbug.com/141716 and 93646
			// Workaround for foundBugs/tabInfo memory leak when the user triggers
			// prefetching/prerendering but never loads the page. Wait two minutes
			// and check whether the tab doesn't exist.
			setTimeout(function () {
				chrome.tabs.get(tab_id, function (tab) {
					if (!tab) {
						log('Clearing orphan tab data for tab %s', tab_id);
						clearTabData(tab_id);
					}
				});
			}, 120000);

			return;
		}

		if (!tabInfo.get(tab_id)) {
			log("tabInfo not found for tab %s, initializing ...", tab_id);

			// TODO expose partialScan in the UI somehow?
			tabInfo.create(tab_id);

			chrome.tabs.get(tab_id, function (tab) {
				var ti = tabInfo.get(tab_id);
				if (ti && ti.partialScan) {
					ti.url = tab.url;
					ti.host = utils.processUrl(tab.url).host;
					ti.incognito = tab.incognito;
				}
			});
		}

		var bug_id,
			app_id,
			page_url = tabInfo.get(tab_id).url,
			incognito = tabInfo.get(tab_id).incognito,
			page_host = tabInfo.get(tab_id).host;

		// NOTE: not currently limiting to certain request types, as before:
		//if (el.nodeName != "EMBED" && el.nodeName != 'IFRAME' && el.nodeName != 'IMG' && el.nodeName != 'OBJECT' && el.nodeName != 'SCRIPT') {
		//	return;
		//}

		bug_id = (page_url ?
			matcher.isBug(deets.url, page_url) :
			matcher.isBug(deets.url));

		if (!bug_id) {
			return;
		}

		app_id = bugDb.db.bugs[bug_id].aid;

		// TODO order of these matters for performance
		var block = !conf.paused_blocking &&

			conf.selected_app_ids.hasOwnProperty(app_id) &&

			// site-specific unblocking
			// page URL might be unavailable
			(!page_host || !conf.site_specific_unblocks.hasOwnProperty(page_host) ||
				conf.site_specific_unblocks[page_host].indexOf(+app_id) == -1) &&

			// page URL might be unavailable
			(!page_url || !whitelisted(page_url)) &&

			!c2pDb.allowedOnce(tab_id, app_id);

		// TODO can URLs repeat within a redirect chain? what are the cases of repeating URLs (trackers only, ...)?
		// latency initialization needs to be synchronous to avoid
		// race condition with onCompleted, etc.
		if (!block && conf.ghostrank) {
			// Store latency data keyed by URL so that we don't use
			// the wrong latencies in a redirect chain.
			latencies[request_id] = latencies[request_id] || {};

			latencies[request_id][deets.url] = {
				start_time: deets.timeStamp,
				bug_id: bug_id,
				// these could be undefined
				page_url: page_url,
				incognito: incognito
			};
		}

		// process the tracker asynchronously
		// v. important to block request processing as little as necessary
		setTimeout(function () {
			processBug({
				bug_id: bug_id,
				app_id: app_id,
				type: deets.type,
				url: deets.url,
				block: block,
				tab_id: tab_id,
				from_frame: deets.parentFrameId != -1
			});

			if (block && conf.ghostrank) {
				// TODO crbug.com/141716 and 93646
				// TODO Handle Omnibox prefetching, which produces requests
				// TODO with tab IDs that do not correspond to a valid tab object.
				chrome.tabs.get(tab_id, function (tab) {
					if (!tab || tab.incognito) {
						return;
					}
					// if bug is blocked, it never loads so latency/rc are redundant: set both to -1.
					ghostrank.record(tab.url, deets.url, bug_id, true, -1, -1, -1, -1);
				});
			}
		}, 1);

		if (block) {
			if (deets.type == 'sub_frame') {
				return { redirectUrl: 'about:blank' };
			} else if (deets.type == 'image') {
				return {
					// send PNG (and not GIF) to avoid conflicts with Adblock Plus
					redirectUrl: 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAACklEQVR4nGMAAQAABQABDQottAAAAABJRU5ErkJggg=='
				};
			} else if (deets.type == 'script') {
				var ti = tabInfo.get(tab_id),
					surrogates = surrogatedb.getForTracker(
						deets.url,
						app_id,
						bug_id,
						ti.host
					);

				if (surrogates.length > 0) {
					var code = _.reduce(surrogates, function (memo, s) {
						memo += s.code;
						return memo;
					}, '');

					var dataUrl = "data:application/javascript;base64," + btoa(code);
					console.log("NEW SURROGATE", app_id);
					return {
						redirectUrl: dataUrl
					};

				}
			}
		}

		return { cancel: block };
	}

	function processBug(deets) {
		var bug_id = deets.bug_id,
			app_id = deets.app_id,
			type = deets.type,
			url = deets.url,
			block = deets.block,
			tab_id = deets.tab_id,
			num_apps_old;

		log('');
		log((block ? 'Blocked' : 'Found') + " [%s] %s", type, url);
		log('^^^ Pattern ID %s on tab ID %s', bug_id, tab_id);

		if (conf.show_alert) {
			num_apps_old = foundBugs.getAppsCount(tab_id);
		}

		foundBugs.update(tab_id, bug_id, url, block, type);

		updateButton(tab_id);

		if (block && conf.enable_click2play) {
			sendC2PData(tab_id, app_id);
		}

		if (conf.show_alert) {
			if (!JUST_UPGRADED || upgrade_alert_shown) {
				if (tabInfo.get(tab_id) && tabInfo.get(tab_id).DOMLoaded) {
					if (foundBugs.getAppsCount(tab_id) > num_apps_old ||
						c2pDb.allowedOnce(tab_id, app_id)) {
						showAlert(tab_id);
					}
				}
			}
		}
	}

	function sendC2PData(tab_id, app_id) {
		var c2pApp = c2pDb.db.apps[app_id];

		if (!c2pApp) {
			return;
		}

		// click-to-play for social buttons might be disabled
		if (!conf.enable_click2play_social) {
			c2pApp = _.reject(c2pApp, function (c2pAppDef) {
				return !!c2pAppDef.button;
			});
		}

		if (!c2pApp.length) {
			return;
		}

		var app_name = bugDb.db.apps[app_id].name,
			c2pHtml = [];

		// generate the templates for each c2p definition (could be multiple for an app ID)
		// TODO move click2play into own file, with maybe the template function embedded in the file?
		c2pApp.forEach(function (c2pAppDef) {
			var tplData = {
				button: !!c2pAppDef.button,
				ghostery_blocked_src: getURL("data/images/click2play/ghosty_blocked.png"),
				allow_always_src: getURL("data/images/click2play/allow_unblock.png")
			};

			if (c2pAppDef.button) {
				tplData.allow_once_title = i18n.t('click2play_allow_once_button_tooltip', app_name);
				tplData.allow_once_src = getURL('data/images/click2play/' + c2pAppDef.button);
			} else {
				tplData.allow_once_title = i18n.t('click2play_allow_once_tooltip');
				tplData.allow_once_src = getURL('data/images/click2play/allow_once.png');

				tplData.ghostery_blocked_title = i18n.t('click2play_blocked', app_name);

				if (c2pAppDef.type) {
					tplData.click2play_text = i18n.t('click2play_' + c2pAppDef.type + '_form', app_name);
				}
			}

			c2pHtml.push(c2p_tpl(tplData));
		});

		// TODO top-level documents only for now: http://crbug.com/264286
		sendMessage(tab_id, 'c2p', {
				app_id: app_id,
				data: c2pApp,
				html: c2pHtml
				//tabWindowId: message.tabWindowId
			});
	}

	function showAlert(tab_id) {
		var apps = foundBugs.getApps(tab_id, true);
		if (apps && apps.length) {
			sendMessage(tab_id, 'show', {
					bugs: apps,
					alert_cfg: {
						pos_x: (conf.alert_bubble_pos.slice(1, 2) == 'r' ? 'right' : 'left'),
						pos_y: (conf.alert_bubble_pos.slice(0, 1) == 't' ? 'top' : 'bottom'),
						timeout: conf.alert_bubble_timeout
					},
					translations: {
						alert_bubble_tooltip: i18n.t('alert_bubble_tooltip'),
						alert_bubble_gear_tooltip: i18n.t('alert_bubble_gear_tooltip')
					}
				});
		}
	}

	function setIcon(active, tab_id) {
		chrome.browserAction.setIcon({
			path: {
				19: 'data/images/icon19' + (active ? '' : '_off') + '.png',
				38: 'data/images/icon38' + (active ? '' : '_off') + '.png'
			},
			tabId: tab_id
		});
	}

	function updateButton(tab_id) {
		var _updateButton = function (tab) {
			if (!tab) {
				return;
			}

			var tab_id = tab.id,
				text;

			if (foundBugs.getBugs(tab_id) === false) {
				// no cached bug discovery data:
				// * Ghostery was enabled after the tab started loading
				// * or, this is a tab onBeforeRequest doesn't run in (non-http/https page)
				text = '';
			} else {
				text = foundBugs.getAppsCount(tab_id).toString();
			}

			chrome.browserAction.setBadgeText({
				text: (conf.show_badge ? text : ''),
				tabId: tab_id
			});

			// gray-out the icon when blocking has been disabled for whatever reason
			if (text === '') {
				setIcon(false, tab_id);
			} else {
				setIcon(!conf.paused_blocking && !whitelisted(tab.url), tab_id);
			}
		};

		if (tab_id) {
			chrome.tabs.get(tab_id, _updateButton);
		} else {
			// no tab ID was provided: update all active tabs
			chrome.tabs.query({
				active: true
			}, function (tabs) {
				tabs.map(_updateButton);
			});
		}
	}

	// does the details object belong to a top-level document and a valid tab?
	function isValidTopLevelNavigation(deets) {
		var url = deets.url;

		return deets.frameId === 0 &&
			deets.tabId > 0 &&
			url.indexOf('http') === 0 &&
			// TODO note this in the "not scanned" text for Chrome
			url.indexOf('https://chrome.google.com/webstore/') !== 0;
	}

	function onNavigation(deets) {
		var tab_id = deets.tabId;

		if (!isValidTopLevelNavigation(deets)) {
			// handle navigation to Web Store from some other page (not from blank page/NTP)
			// TODO what other webRequest-restricted pages are out there?
			if (deets.url.indexOf('https://chrome.google.com/webstore/') === 0) {
				clearTabData(tab_id);
			}

			return;
		}

		c2pDb.reset(tab_id);

		// note that we were here from the start (not scanned vs. nothing found)
		foundBugs.update(tab_id);

		updateButton(tab_id);
	}

	function init() {
		// initialize trackers and surrogates
		bugDb.init(JUST_UPGRADED);
		c2pDb.init(JUST_UPGRADED);
		compDb.init(JUST_UPGRADED);
		tagDb.init(JUST_UPGRADED);

		i18n.init(conf.language);

		// webNavigation events are closely related to navigation state in the UI,
		// which makes them better suited for updating the browser button and badge
		chrome.webNavigation.onCommitted.addListener(onNavigation);
		chrome.webNavigation.onReferenceFragmentUpdated.addListener(onNavigation);
		if (chrome.webNavigation.onHistoryStateUpdated) {
			chrome.webNavigation.onHistoryStateUpdated.addListener(onNavigation);
		}
		// TODO onCreatedNavigationTarget?
		// TODO onTabReplaced?

		// Listen for completed webRequests. Calculate latency and send GR if enabled.
		chrome.webRequest.onCompleted.addListener(function (deets) {
			logLatency(deets, true);
		}, {
			urls: ['http://*/*', 'https://*/*']
		});

		// Listen for redirects. Calculate latency and send GR if enabled.
		chrome.webRequest.onBeforeRedirect.addListener(function (deets) {
			logLatency(deets, true);
		}, {
			urls: ['http://*/*', 'https://*/*']
		});

		// Listen for error webRequests. Set latency = -1 and send GR if enabled.
		chrome.webRequest.onErrorOccurred.addListener(function (deets) {
			logLatency(deets, false);
		}, {
			urls: ['http://*/*', 'https://*/*']
		});

		chrome.webRequest.onBeforeRequest.addListener(onBeforeRequest, {
			urls: ['http://*/*', 'https://*/*']
		}, ['blocking']);

		chrome.browserAction.setBadgeBackgroundColor({ color: [51, 0, 51, 230] });

		chrome.tabs.onRemoved.addListener(clearTabData);
		chrome.tabs.onActiveChanged.addListener(function (tab_id) {
			updateButton(tab_id);
		});

		onMessage.addListener(function (request, sender, sendResponse) {
			var name = request.name,
				message = request.message,
				tab = sender.tab,
				tab_id = tab && tab.id,
				tab_url = tab && tab.url;

			return injectedScriptMessageListener(name, message, tab_id, tab_url, sendResponse);
		});

		chrome.webNavigation.onDOMContentLoaded.addListener(function (deets) {
			var tab_id = deets.tabId;

			if (!isValidTopLevelNavigation(deets)) {
				return;
			}

			onDOMContentLoaded(tab_id);
		});

		// messaging with Ghostery UI pages
		initDispatcher();

		setInterval(function () {
			autoUpdateBugDb();
		}, 300000); // run every five minutes

		if (!!(!conf.ghostrank && !prefs('walkthroughAborted') && !prefs('walkthroughFinished'))) {
			openTab('walkthrough.html', true);
		}
	}

	init();

});
