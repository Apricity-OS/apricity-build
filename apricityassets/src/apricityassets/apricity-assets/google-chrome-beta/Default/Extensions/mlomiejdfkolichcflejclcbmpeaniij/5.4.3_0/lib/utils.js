/*!
 * Ghostery for Chrome
 * http://www.ghostery.com/
 *
 * Copyright 2014 Ghostery, Inc. All rights reserved.
 * See https://www.ghostery.com/eula for license.
 */

define([
	'underscore',
	'jquery'
], function (_, $) {

	var utils = {};

	utils.syncGet = function (url) {
		var xhr = new XMLHttpRequest();
		xhr.open('GET', url, false);
		xhr.send(null);
		return xhr.responseText;
	};

	utils.VERSION = JSON.parse(utils.syncGet(chrome.extension.getURL('manifest.json'))).version;

	utils.log = function () {
		if (!localStorage.debug) {
			return;
		}
		// convert arguments object to array
		var args = [].slice.call(arguments, 0);
		if (args[0]) {
			args[0] = (new Date()).toTimeString() + '\t\t' + args[0];
		}
		console.log.apply(console, args);
	};

	utils.prefs = function (key, value) {
		if (typeof value != 'undefined') {
			localStorage.setItem(key, JSON.stringify(value));
		} else {
			value = localStorage.getItem(key);
			return value && JSON.parse(value);
		}
	};

	utils.createLink = function (href, text, class_name, title) {
		var link = document.createElement('a');
		link.href = href;
		link.target = '_blank';
		if (class_name) {
			link.className = class_name;
		}
		if (title) {
			link.title = title;
		}
		link.appendChild(document.createTextNode(text));
		return link;
	};

	utils.createCheckbox = function (id, checked, class_name) {
		var check = document.createElement('input');
		check.type = 'checkbox';
		if (checked !== undefined) {
			check.checked = !!checked;
		}
		if (class_name) {
			check.className = class_name;
		}
		check.id = id;
		return check;
	};

	utils.getActiveTab = function (callback) {
		chrome.tabs.query({
			active: true,
			windowId: chrome.windows.WINDOW_ID_CURRENT
		}, function (tabs) {
			callback(tabs[0]);
		});
	};

	// Modified code from: http://werxltd.com/wp/2010/05/13/javascript-implementation-of-javas-string-hashcode-method/
	// Used for checking the hash of the backup file, easy validation check
	utils.hashCode = function (str) {
		var hash = 0,
			character,
			i;

		if (str.length === 0) {
			return hash;
		}

		for (i = 0; i < str.length; i++) {
			character = str.charCodeAt(i);
			hash = ((hash << 5) - hash) + character; // jshint ignore:line
			hash = hash & hash; // jshint ignore:line
		}

		return hash;
	};

	// Flush Chrome's in-memory cache.
	// GHOST-248: Debounced to run no more often than once every 35 seconds
	// to avoid the "slow extension" warning.
	// TODO should run once at start of period, and once at end?
	// TODO use chrome.webRequest.MAX_HANDLER_BEHAVIOR_CHANGED_CALLS_PER_10_MINUTES (can default to 20 if undefined)
	utils.flushChromeMemoryCache = _.debounce(function () {
		utils.log("FLUSHING CHROME'S IN-MEMORY CACHE");
		chrome.webRequest.handlerBehaviorChanged();
	}, 1000 * 35, true);

	utils.fuzzyUrlMatcher = function (url, urls) {
		// TODO belongs in tabInfo
		var parsed = utils.processUrl(url),
			tab_host = parsed.host,
			tab_path = parsed.path;

		if (tab_host.indexOf('www.') === 0) {
			tab_host = tab_host.slice(4);
		}

		for (var i = 0; i < urls.length; i++) {
			parsed = utils.processUrl(urls[i]);
			var host = parsed.host,
				path = parsed.path;

			if (host != tab_host) {
				continue;
			}

			if (!path) {
				utils.log('[fuzzyUrlMatcher] host (%s) match', host);
				return true;
			}

			if (path.slice(-1) == '*') {
				if (tab_path.indexOf(path.slice(0, -1)) === 0) {
					utils.log('[fuzzyUrlMatcher] host (%s) and path (%s) fuzzy match', host, path);
					return true;
				}

			} else {
				if (path == tab_path) {
					utils.log('[fuzzyUrlMatcher] host (%s) and path (%s) match', host, path);
					return true;
				}
			}
		}
	};

	// TODO review applicability to lib/conf
	utils.defineLazyProperty = function (o, name, fun) {
		var value,
			isSet = false;

		Object.defineProperty(o, name, {
			get: function () {
				if (!isSet) {
					value = fun();
					isSet = true;
				}

				return value;
			},

			set: function (v) {
				value = v;
				isSet = true;
			}
		});
	};

	// We need to deep clone objects/arrays with other objects
	// arrays inside of them or the current/old check won't work
	// since the inner objects are references instead of copies.
	// Also just in case the value is altered whereever you send it.
	utils.deepClone = function (v) {
		if (_.isObject(v)) {
			if (_.isArray(v)) {
				return $.extend(true, [], v);
			} else {
				return $.extend(true, {}, v);
			}
		} else {
			// cloning doesn't make sense for non-objects
			return v;
		}
	};

	utils.processUrl = function (src) {
		var q,
			r,
			anchor,
			src_host,
			src_path,
			src_cleaned,
			src_protocol = '';

		// strip out the hash
		q = src.indexOf('#');
		if (q >= 0) {
			anchor = src.slice(q + 1);
			src = src.slice(0, q);
		}

		// strip out the querystring, including the ?, to reduce false positives
		q = src.indexOf('?');
		if (q >= 0) {
			src = src.slice(0, q);
		}

		// original src without querystring and hash
		src_cleaned = src;

		// strip out the scheme
		q = src.indexOf('http://');
		if (q === 0) {
			src_protocol = src.substr(0, 4);
			src = src.slice(7);
		} else {
			q = src.indexOf('https://');
			if (q === 0) {
				src_protocol = src.substr(0, 5);
				src = src.slice(8);
			} else {
				// protocol-relative URLs
				q = src.indexOf('//');
				if (q === 0) {
					src = src.slice(2);
				}
			}
		}

		src = src.toLowerCase();

		q = src.indexOf('/');

		// check for @ in host, http://google.com:xxx@evil.com will show as google.com otherwise
		r = src.indexOf("@");

		if (r >= 0 && (q === -1 || r < q)) {
			src = src.slice(r + 1);
			// grab '/' loc again since the src changed
			q = src.indexOf('/');
		}

		// host should be everything from the start until the first "/"
		src_host = (q >= 0 ? src.substr(0, q) : src);
		// path should be everything after the first "/"
		src_path = (q >= 0 ? src.substr(q + 1) : '');

		// remove port from src_host
		q = src_host.indexOf(':');
		if (q >= 0) {
			src_host = src_host.substr(0, q);
		}

		return {
			protocol: src_protocol,
			host: src_host,
			path: src_path,
			host_with_path: src,
			anchor: anchor,
			// NOTE: special case required by ghostrank where we keep
			// original casing and strip out querystring and hash
			host_with_path_cleaned: src_cleaned
		};
	};

	return utils;

});
